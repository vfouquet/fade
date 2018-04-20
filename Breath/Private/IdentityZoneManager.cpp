// Fill out your copyright notice in the Description page of Project Settings.

#include "IdentityZoneManager.h"

#include "Components/PostProcessComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "IdentityEraserComponent.h"
#include "MemoryZoneComponent.h"
#include "Engine/World.h"
#include "Components/DecalComponent.h"
#include "Components/BoxComponent.h"

// Sets default values
AIdentityZoneManager::AIdentityZoneManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AIdentityZoneManager::BeginPlay()
{
	Super::BeginPlay();
	
	UPostProcessComponent*	postProcessComponent = FindComponentByClass<UPostProcessComponent>();
	if (!postProcessComponent || !materialInterface)
		return;

	whiteZoneMaterial = UMaterialInstanceDynamic::Create(materialInterface, this);
	
	postProcessComponent->Settings.AddBlendable(whiteZoneMaterial, 1.0f);
	if (auto* cont = GetWorld()->GetFirstPlayerController())
		character = cont->GetPawn();
	
	UBoxComponent*	boxComp = FindComponentByClass<UBoxComponent>();
	if (!boxComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s - Couldn't find attach box component"), *GetName());
		return;
	}
	FVector boxExtent = boxComp->GetScaledBoxExtent();

	if (DecalNormalMaterialInterface)
		createNormalDecal(boxExtent);
	if (DecalRoughnessMaterialInterface)
		createRoughnessDecal(boxExtent);
}

// Called every frame
void AIdentityZoneManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (auto* cont = GetWorld()->GetFirstPlayerController())
		character = cont->GetPawn();

	affectedObjects.RemoveAll([](const FErasedObjectProperties& p1) { return !p1.primitiveComponent.IsValid(); });
	for (auto& erasedObject : affectedObjects)
	{
		if (erasedObject.interactableComponent.IsValid() && erasedObject.interactableComponent->MemoryInteractable)
			continue;
		if (!erasedObject.bDecelerating)
			continue;
		erasedObject.currentDecelerationTime += DeltaTime;
		if (erasedObject.currentDecelerationTime >= erasedObject.maxDeceleratingTime)
		{
			erasedObject.bDecelerating = false;
			erasedObject.primitiveComponent->ComponentVelocity = FVector::ZeroVector;
			erasedObject.primitiveComponent->SetSimulatePhysics(false);
			if (erasedObject.interactableComponent.IsValid())
				erasedObject.interactableComponent->EraseIdentity();
		}
		else
			erasedObject.primitiveComponent->SetPhysicsLinearVelocity(FMath::Lerp(erasedObject.initialVelocity, FVector::ZeroVector,
				erasedObject.currentDecelerationTime / erasedObject.maxDeceleratingTime));
	}

	if (!whiteZoneMaterial || !decalNormalMaterial || !decalRoughnessMaterial)
		return;

	if (!character)
		return;

	erasedZones.RemoveAll([](const TWeakObjectPtr<UIdentityEraserComponent>& p1) { return !p1.IsValid(); });

	FVector sourceLocation = character->GetActorLocation();
	erasedZones.Sort([sourceLocation](const TWeakObjectPtr<UIdentityEraserComponent>& A, const TWeakObjectPtr<UIdentityEraserComponent>& B)
	{
		float distA = FVector::DistSquared(sourceLocation, A->GetComponentLocation());
		float distB = FVector::DistSquared(sourceLocation, B->GetComponentLocation());
		return distA < distB;
	});

	int maxIdx = erasedZones.Num();
	if (maxIdx > 30)
		maxIdx = 30;

	int pos = 0;
	for (pos; pos < maxIdx; pos++)
	{
		FString indexStr = FString::FromInt(pos + 1);

		whiteZoneMaterial->SetScalarParameterValue(FName(*("Size_" + indexStr)), erasedZones[pos]->GetScaledSphereRadius() + ZoneOffset * 2.0f);
		whiteZoneMaterial->SetVectorParameterValue(FName(*("Position_" + indexStr)), erasedZones[pos]->GetComponentLocation());
	
		decalNormalMaterial->SetScalarParameterValue(FName(*("Size_" + indexStr)), erasedZones[pos]->GetScaledSphereRadius() + ZoneOffset * 2.0f);
		decalNormalMaterial->SetVectorParameterValue(FName(*("Position_" + indexStr)), erasedZones[pos]->GetComponentLocation());

		decalRoughnessMaterial->SetScalarParameterValue(FName(*("Size_" + indexStr)), erasedZones[pos]->GetScaledSphereRadius() + ZoneOffset * 2.0f);
		decalRoughnessMaterial->SetVectorParameterValue(FName(*("Position_" + indexStr)), erasedZones[pos]->GetComponentLocation());
	}
	for (pos; pos < 30; pos++)
	{
		FString indexStr = FString::FromInt(pos + 1);
		whiteZoneMaterial->SetScalarParameterValue(FName(*("Size_" + indexStr)), 0.0f);

		decalNormalMaterial->SetScalarParameterValue(FName(*("Size_" + indexStr)), 0.0f);
		
		decalRoughnessMaterial->SetScalarParameterValue(FName(*("Size_" + indexStr)), 0.0f);
	}

	memoryZones.RemoveAll([](const TWeakObjectPtr<UMemoryZoneComponent>& p1) { return !p1.IsValid(); });

	memoryZones.Sort([sourceLocation](const TWeakObjectPtr<UMemoryZoneComponent>& A, const TWeakObjectPtr<UMemoryZoneComponent>& B)
	{
		float distA = FVector::DistSquared(sourceLocation, A->GetComponentLocation());
		float distB = FVector::DistSquared(sourceLocation, B->GetComponentLocation());
		return distA < distB;
	});

	maxIdx = memoryZones.Num();
	if (maxIdx > 5)
		maxIdx = 5;

	pos = 0;
	for (pos; pos < maxIdx; pos++)
	{
		FString indexStr = FString::FromInt(pos + 1);

		whiteZoneMaterial->SetScalarParameterValue(FName(*("Memory_Size_" + indexStr)), memoryZones[pos]->GetScaledSphereRadius());
		whiteZoneMaterial->SetVectorParameterValue(FName(*("Memory_Position_" + indexStr)), memoryZones[pos]->GetComponentLocation());
	
		decalNormalMaterial->SetScalarParameterValue(FName(*("Memory_Size_" + indexStr)), memoryZones[pos]->GetScaledSphereRadius());
		decalNormalMaterial->SetVectorParameterValue(FName(*("Memory_Position_" + indexStr)), memoryZones[pos]->GetComponentLocation());

		decalRoughnessMaterial->SetScalarParameterValue(FName(*("Memory_Size_" + indexStr)), memoryZones[pos]->GetScaledSphereRadius());
		decalRoughnessMaterial->SetVectorParameterValue(FName(*("Memory_Position_" + indexStr)), memoryZones[pos]->GetComponentLocation());
	}
	for (pos; pos < 5; pos++)
	{
		FString indexStr = FString::FromInt(pos + 1);
		whiteZoneMaterial->SetScalarParameterValue(FName(*("Memory_Size_" + indexStr)), 0.0f);
		
		decalNormalMaterial->SetScalarParameterValue(FName(*("Memory_Size_" + indexStr)), 0.0f);
		
		decalRoughnessMaterial->SetScalarParameterValue(FName(*("Memory_Size_" + indexStr)), 0.0f);
	}
}

void	AIdentityZoneManager::RemoveZone(UIdentityEraserComponent* value)
{
	erasedZones.Remove(value);
}

void	AIdentityZoneManager::RemoveZone(UMemoryZoneComponent* value)
{
	memoryZones.Remove(value);
}

AIdentityZoneManager::FErasedObjectProperties&	AIdentityZoneManager::createNewProperties(UPrimitiveComponent* primitiveComponent, float decelerationTime)
{
	FErasedObjectProperties	properties;
	properties.primitiveComponent = primitiveComponent;

	UInteractableComponent* interactableComp = UInteractableComponent::FindAssociatedInteractableComponent(primitiveComponent);
	if (interactableComp)
	{
		properties.interactableComponent = interactableComp;
		if (interactableComp->MemoryInteractable)
		{
			int32 idx = affectedObjects.Add(properties);
			return affectedObjects[idx];
		}
	}

	UIdentityPhysicsOverrideComponent*	physicsOverrider = UIdentityPhysicsOverrideComponent::FindAssociatedPhysicsOverriderComponent(primitiveComponent);
	if (physicsOverrider)
	{
		properties.physicsOverrider = physicsOverrider;
		physicsOverrider->EraseIdentity();
	}
	else
	{
		properties.bWasSimulatingPhysics = primitiveComponent->IsSimulatingPhysics();
		if (properties.bWasSimulatingPhysics)
		{
			properties.initialVelocity = primitiveComponent->GetComponentVelocity();
			properties.bDecelerating = true;
			properties.maxDeceleratingTime = decelerationTime;
		}
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

AIdentityZoneManager::FErasedObjectProperties*	AIdentityZoneManager::containsErasedObjectProperties(UPrimitiveComponent* reference, bool& foundSomething, int& outID)
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

void	AIdentityZoneManager::updateObjectProperties(AIdentityZoneManager::FErasedObjectProperties& properties)
{
	if (properties.interactableComponent.IsValid())
	{
		if (properties.interactableComponent->MemoryInteractable)
			return;
		properties.interactableComponent->EraseIdentity();
	}

	if (!properties.physicsOverrider.IsValid())
	{
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
	}

	if (properties.chemicalComponent.IsValid())
	{
		properties.previousChemicalState = properties.chemicalComponent->GetState();
		properties.chemicalComponent->EraseIdentity();
	}
}

void	AIdentityZoneManager::updateObjectProperties(AIdentityZoneManager::FErasedObjectProperties& properties, float decelerationTime)
{
	if (properties.interactableComponent.IsValid())
	{
		if (properties.interactableComponent->MemoryInteractable)
			return;
		properties.interactableComponent->EraseIdentity();
	}

	if (!properties.physicsOverrider.IsValid())
	{
		properties.bWasSimulatingPhysics = properties.primitiveComponent->IsSimulatingPhysics();
		if (properties.bWasSimulatingPhysics)
		{
			properties.initialVelocity = properties.primitiveComponent->GetComponentVelocity();
			properties.bDecelerating = true;
			properties.maxDeceleratingTime = decelerationTime;
		}
	}

	if (properties.chemicalComponent.IsValid())
	{
		properties.previousChemicalState = properties.chemicalComponent->GetState();
		properties.chemicalComponent->EraseIdentity();
	}
}

void	AIdentityZoneManager::createNormalDecal(FVector decalExtent)
{
	UDecalComponent* decal = NewObject<UDecalComponent>(this, FName("NormalDecal"));
	if (!decal)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s : Couldn't create normal decal component"), *GetName());
		return;
	}
	decal->SetupAttachment(GetRootComponent());
	decal->RegisterComponent();
	decal->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f).Quaternion());
	decal->DecalSize = decalExtent;
	decalNormalMaterial = UMaterialInstanceDynamic::Create(DecalNormalMaterialInterface, this);
	if (decalNormalMaterial)
		decal->SetDecalMaterial(decalNormalMaterial);
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s : Couldn't create decal normal material"), *GetName());
	}
}

void	AIdentityZoneManager::createRoughnessDecal(FVector decalExtent)
{
	UDecalComponent* decal = NewObject<UDecalComponent>(this, FName("RoughnessDecal"));
	if (!decal)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s : Couldn't create roughness decal component"), *GetName());
		return;
	}
	decal->SetupAttachment(GetRootComponent());
	decal->RegisterComponent();
	decal->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f).Quaternion());
	decal->DecalSize = decalExtent;
	decalRoughnessMaterial = UMaterialInstanceDynamic::Create(DecalRoughnessMaterialInterface, this);
	if (decalRoughnessMaterial)
		decal->SetDecalMaterial(decalRoughnessMaterial);
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s : Couldn't create decal roughness material"), *GetName());
	}
}