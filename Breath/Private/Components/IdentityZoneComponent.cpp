// Fill out your copyright notice in the Description page of Project Settings.

#include "IdentityZoneComponent.h"

#include "Engine/World.h"
#include "IdentityEraserComponent.h"

TArray<UIdentityZoneComponent::FErasedObjectProperties>	
	UIdentityZoneComponent::affectedObjects = TArray<UIdentityZoneComponent::FErasedObjectProperties>();

void	UIdentityZoneComponent::BeginPlay()
{
	Super::BeginPlay();

	FScriptDelegate	beginOverlapDelegate;
	beginOverlapDelegate.BindUFunction(this, "OnBeginOverlap");
	OnComponentBeginOverlap.AddUnique(beginOverlapDelegate);

	FScriptDelegate	endOverlapDelegate;
	endOverlapDelegate.BindUFunction(this, "OnEndOverlap");
	OnComponentEndOverlap.AddUnique(endOverlapDelegate);

	TArray<UPrimitiveComponent*>	primitives;
	GetOverlappingComponents(primitives);

	for (auto& prim : primitives)
	{
		AActor* otherActor = prim->GetOwner();
		if (otherActor != GetOwner())
			OnBeginOverlap(nullptr, otherActor, prim, 0, false, FHitResult()); //DANGEROUS SHIT
	}
}

void UIdentityZoneComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	affectedObjects.RemoveAll([](const FErasedObjectProperties& p1) { return p1.primitiveComponent.IsStale(); });
	for (auto& erasedObject : affectedObjects)
	{
		if (GetWorld()->RealTimeSeconds == erasedObject.lastTimeUpdated)
			continue;
		if (erasedObject.interactableComponent.IsValid() && erasedObject.interactableComponent->MemoryInteractable)
			continue;
		
		erasedObject.lastTimeUpdated = GetWorld()->RealTimeSeconds;
		if (!erasedObject.bDecelerating)
			continue;
		erasedObject.currentDecelerationTime += DeltaTime;
		if (erasedObject.currentDecelerationTime >= erasedObject.maxDeceleratingTime)
		{
			erasedObject.bDecelerating = false;
			erasedObject.primitiveComponent->ComponentVelocity = FVector::ZeroVector;
			erasedObject.primitiveComponent->SetSimulatePhysics(false);
		}
		else
			erasedObject.primitiveComponent->SetPhysicsLinearVelocity(FMath::Lerp(erasedObject.initialVelocity, FVector::ZeroVector,
				erasedObject.currentDecelerationTime / erasedObject.maxDeceleratingTime));
	}
}

UIdentityZoneComponent::FErasedObjectProperties&	UIdentityZoneComponent::createNewProperties(UPrimitiveComponent* primitiveComponent, float decelerationTime)
{
	FErasedObjectProperties	properties;
	properties.primitiveComponent = primitiveComponent;
	
	UInteractableComponent* interactableComp = UInteractableComponent::FindAssociatedInteractableComponent(primitiveComponent);
	if (interactableComp && interactableComp->MemoryInteractable)
	{
		properties.interactableComponent = interactableComp;
		if (interactableComp->MemoryInteractable)
		{
			int32 idx = affectedObjects.Add(properties);
			return affectedObjects[idx];
		}
		interactableComp->EraseIdentity();
	}

	properties.bWasSimulatingPhysics = primitiveComponent->IsSimulatingPhysics();
	if (properties.bWasSimulatingPhysics)
	{
		properties.initialVelocity = primitiveComponent->GetComponentVelocity();
		properties.bDecelerating = true;
		properties.maxDeceleratingTime = decelerationTime;
	}

	UChemicalComponent*	chemicalComp = UChemicalComponent::FindAssociatedChemicalComponent(primitiveComponent);
	if (chemicalComp)
	{
		properties.previousChemicalState = chemicalComp->GetState();
		chemicalComp->EraseIdentity();
		properties.chemicalComponent = chemicalComp;
	}
	
	int32 idx = affectedObjects.Add(properties);
	return affectedObjects[idx];
}

UIdentityZoneComponent::FErasedObjectProperties*	UIdentityZoneComponent::containsErasedObjectProperties(UPrimitiveComponent* reference, bool& foundSomething, int& outID)
{
	for (int pos = 0; pos < affectedObjects.Num(); pos++)
	{
		if (affectedObjects[pos].primitiveComponent == reference)
		{
			foundSomething = true;
			outID = pos;
			return &affectedObjects[pos];
		}
	}
	foundSomething = false;
	return nullptr;
}

void	UIdentityZoneComponent::updateObjectProperties(UIdentityZoneComponent::FErasedObjectProperties& properties)
{
	if (properties.interactableComponent.IsValid())
	{
		if (properties.interactableComponent->MemoryInteractable)
			return;
		properties.interactableComponent->EraseIdentity();
	}

	properties.bWasSimulatingPhysics = properties.primitiveComponent->IsSimulatingPhysics();
	if (properties.bWasSimulatingPhysics)
	{
		properties.initialVelocity = properties.primitiveComponent->GetComponentVelocity();
		properties.bDecelerating = true;
		
		TArray<UPrimitiveComponent*>	actorComponents;
		properties.primitiveComponent->GetOverlappingComponents(actorComponents);
		float	maxTime = FLT_MAX;
		for (auto& primitiveComp : actorComponents)
		{
			UIdentityEraserComponent* eraserZone = Cast<UIdentityEraserComponent>(primitiveComp);
			if (eraserZone && eraserZone->DecelerationTime <= maxTime)
				maxTime = eraserZone->DecelerationTime;
		}
		if (maxTime != FLT_MAX)
			properties.maxDeceleratingTime = maxTime;
	}

	if (properties.chemicalComponent.IsValid())
	{
		properties.previousChemicalState = properties.chemicalComponent->GetState();
		properties.chemicalComponent->EraseIdentity();
	}
}

void	UIdentityZoneComponent::updateObjectProperties(UIdentityZoneComponent::FErasedObjectProperties& properties, float decelerationTime)
{
	if (properties.interactableComponent.IsValid())
	{
		if (properties.interactableComponent->MemoryInteractable)
			return;
		properties.interactableComponent->EraseIdentity();
	}

	properties.bWasSimulatingPhysics = properties.primitiveComponent->IsSimulatingPhysics();
	if (properties.bWasSimulatingPhysics)
	{
		properties.initialVelocity = properties.primitiveComponent->GetComponentVelocity();
		properties.bDecelerating = true;
		properties.maxDeceleratingTime = decelerationTime;
	}

	if (properties.chemicalComponent.IsValid())
	{
		properties.previousChemicalState = properties.chemicalComponent->GetState();
		properties.chemicalComponent->EraseIdentity();
	}
}