// Fill out your copyright notice in the Description page of Project Settings.

#include "MemoryZoneComponent.h"

#include "GameFramework/Character.h"
#include "IdentityZoneManager.h"

void	UMemoryZoneComponent::BeginPlay()
{
	Super::BeginPlay();

	if (manager)
		manager->AddMemoryZone(this);
	else
	{
		AActor* owner = GetOwner();
		UE_LOG(LogTemp, Warning, TEXT("%s - MemoryZoneComponent : Identity Zone Manager doesn't exist"), owner ? *owner->GetName() : *FString("Error"));
	}

	if (MemoryVolumeMesh && MemoryOpaqueMaterial && MemoryTransparentMaterial)
	{
		UStaticMeshComponent*	opaqueSphere = NewObject<UStaticMeshComponent>(this);
		UStaticMeshComponent*	transparentSphere = NewObject<UStaticMeshComponent>(this);
		if (!opaqueSphere || !transparentSphere)
		{
			AActor* owner = GetOwner();
			UE_LOG(LogTemp, Warning, TEXT("%s - MemoryZoneComponent : Sphere not created"), owner ? *owner->GetName() : *FString("Error"));
		}

		opaqueSphere->SetupAttachment(this);
		transparentSphere->SetupAttachment(this);
		opaqueSphere->RegisterComponent();
		transparentSphere->RegisterComponent();
		transparentSphere->SetStaticMesh(MemoryVolumeMesh);
		opaqueSphere->SetStaticMesh(MemoryVolumeMesh);
		transparentSphere->SetMaterial(0, MemoryTransparentMaterial);
		opaqueSphere->SetMaterial(0, MemoryOpaqueMaterial);
		opaqueSphere->bRenderCustomDepth = true;
		opaqueSphere->CustomDepthStencilValue = 252;
		opaqueSphere->bRenderInMainPass = false;
		opaqueSphere->SetSimulatePhysics(false);
		opaqueSphere->SetCollisionProfileName("NoCollision");
		transparentSphere->SetSimulatePhysics(false);
		transparentSphere->SetCollisionProfileName("NoCollision");

		float scale = GetUnscaledSphereRadius() / MemoryVolumeMesh->ExtendedBounds.BoxExtent.X;
		opaqueSphere->SetRelativeScale3D(FVector(scale));
		transparentSphere->SetRelativeScale3D(FVector(scale));
	}
	else
	{
		AActor* owner = GetOwner();
		FString meshState = MemoryVolumeMesh ? "OK" : "nullptr";
		FString materialState = MemoryOpaqueMaterial ? "OK" : "nullptr";
		FString materialTransparentState = MemoryTransparentMaterial ? "OK" : "nullptr";
		UE_LOG(LogTemp, Warning,
			TEXT("%s - MemoryZoneComponent : Cannot create sphere volume, sphere mesh state is %s, opaque material state is %s and transparent state is %s"),
			owner ? *owner->GetName() : *FString("Error"), *meshState, *materialState, *materialTransparentState);
	}
}
	
void	UMemoryZoneComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	Super::OnComponentDestroyed(bDestroyingHierarchy);

	if (manager)
		manager->RemoveZone(this);
}

void	UMemoryZoneComponent::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (!manager)
		return;

	if (ACharacter* character = Cast<ACharacter>(OtherActor))
		return;

	int id = 0;
	bool isPropertyExisting = false;
	AIdentityZoneManager::FErasedObjectProperties* outProperty = manager->containsErasedObjectProperties(OtherComp, isPropertyExisting, id);
	if (isPropertyExisting)
	{
		if (outProperty->memoryZoneNbr == 0 && outProperty->erasedZoneNbr > 0)
		{
			if (outProperty->physicsOverrider.IsValid())
				outProperty->physicsOverrider->GiveIdentity();
			else
			{
				if (outProperty->bWasSimulatingPhysics)
					OtherComp->SetSimulatePhysics(true);
			}
			if (outProperty->chemicalComponent.IsValid())
				outProperty->chemicalComponent->GiveIdentity(outProperty->previousChemicalState);
			if (outProperty->interactableComponent.IsValid())
				outProperty->interactableComponent->GiveIdentity();
			outProperty->bDecelerating = false;
		}
		outProperty->memoryZoneNbr++;
	}
	else
	{
		if (Cast<ACharacter>(OtherActor))
			return;
		auto& properties = manager->createNewProperties(OtherComp);
		properties.bDecelerating = false;
		properties.memoryZoneNbr++;
	}
}

void	UMemoryZoneComponent::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!manager)
		return;

	if (ACharacter* character = Cast<ACharacter>(OtherActor))
		return;

	int id = 0;
	bool isPropertyExisting = false;
	AIdentityZoneManager::FErasedObjectProperties* outProperty = manager->containsErasedObjectProperties(OtherComp, isPropertyExisting, id);
	if (isPropertyExisting)
	{
		if (outProperty->memoryZoneNbr == 1)
		{
			if (outProperty->erasedZoneNbr > 0)
			{
				manager->updateObjectProperties(*outProperty);
				outProperty->memoryZoneNbr = 0;
			}
			else
				manager->RemoveAffectedObject(id);
		}
		else
			outProperty->memoryZoneNbr--;
	}
}