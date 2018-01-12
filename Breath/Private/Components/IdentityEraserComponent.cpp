// Fill out your copyright notice in the Description page of Project Settings.

#include "IdentityEraserComponent.h"
#include "GameFramework/Character.h"

#include "ChemicalComponent.h"
#include "InteractableComponent.h"

// Sets default values for this component's properties
UIdentityEraserComponent::UIdentityEraserComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void	UIdentityEraserComponent::BeginPlay()
{
	Super::BeginPlay();

	FScriptDelegate	beginOverlapDelegate;
	beginOverlapDelegate.BindUFunction(this, "OnBeginOverlap");
	OnComponentBeginOverlap.AddUnique(beginOverlapDelegate);

	FScriptDelegate	endOverlapDelegate;
	endOverlapDelegate.BindUFunction(this, "OnEndOverlap");
	OnComponentEndOverlap.AddUnique(endOverlapDelegate);
}

void UIdentityEraserComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	for (auto& erasedObject : erasedObjects)
	{	
		if (!erasedObject.decelerating)
			continue;
		erasedObject.currentDecelerationTime += DeltaTime;
		if (erasedObject.currentDecelerationTime >= DecelerationTime)
		{
			erasedObject.decelerating = false;
			erasedObject.primitiveComponent->ComponentVelocity = FVector::ZeroVector;
			erasedObject.primitiveComponent->SetSimulatePhysics(false);
		}
		else
			erasedObject.primitiveComponent->SetPhysicsLinearVelocity(FMath::Lerp(erasedObject.initialVelocity, FVector::ZeroVector, 
				erasedObject.currentDecelerationTime / DecelerationTime));
	}
}

void	UIdentityEraserComponent::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (Cast<ACharacter>(OtherActor))
		return;
	FErasedObjectProperties	properties;
	properties.primitiveComponent = OtherComp;
	properties.wasSimulatingPhysics = OtherComp->IsSimulatingPhysics();
	if (properties.wasSimulatingPhysics)
	{
		properties.initialVelocity = OtherComp->GetComponentVelocity();
		properties.decelerating = true;
	}

	UChemicalComponent*	chemicalComp = UChemicalComponent::FindAssociatedChemicalComponent(OtherComp);
	if (chemicalComp)
	{
		properties.previousChemicalState = chemicalComp->GetState();
		chemicalComp->EraseIdentity();
		properties.chemicalComponent = chemicalComp;
	}
	TArray<UActorComponent*>	interactableComponents = OtherActor->GetComponentsByClass(UInteractableComponent::StaticClass());
	for (auto& actorComp : interactableComponents)
	{
		UInteractableComponent* interactableComp = Cast<UInteractableComponent>(actorComp);
		if (!interactableComp)	continue;
		interactableComp->EraseIdentity();
	}
	erasedObjects.Add(properties);
}

void	UIdentityEraserComponent::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	for (int pos = 0; pos < erasedObjects.Num(); pos++)
	{
		FErasedObjectProperties&	properties = erasedObjects[pos];
		if (properties.primitiveComponent != OtherComp)
			continue;
		if (properties.wasSimulatingPhysics)
			OtherComp->SetSimulatePhysics(true);
		if (properties.chemicalComponent.IsValid())
			properties.chemicalComponent->GiveIdentity(properties.previousChemicalState);
		erasedObjects.RemoveAt(pos);
		break;
	}
	TArray<UActorComponent*>	interactableComponents = OtherActor->GetComponentsByClass(UInteractableComponent::StaticClass());
	for (auto& actorComp : interactableComponents)
	{
		UInteractableComponent* interactableComp = Cast<UInteractableComponent>(actorComp);
		if (!interactableComp)	continue;
		interactableComp->GiveIdentity();
	}
}