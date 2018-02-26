// Fill out your copyright notice in the Description page of Project Settings.

#include "IdentityEraserComponent.h"
#include "GameFramework/Character.h"

#include "ChemicalComponent.h"
#include "InteractableComponent.h"
#include "IdentityZoneManager.h"

void	UIdentityEraserComponent::BeginPlay()
{
	Super::BeginPlay();

	if (manager)
		erasedIndex = manager->AddErasedZone(this);
}

void	UIdentityEraserComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	if (manager)
		manager->RemoveZone(erasedIndex);
}

void	UIdentityEraserComponent::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	bool isPropertyExisting = false;
	int id = 0;
	AIdentityZoneManager::FErasedObjectProperties* outProperty = manager->containsErasedObjectProperties(OtherComp, isPropertyExisting, id);
	if (isPropertyExisting)
	{
		if (outProperty->erasedZoneNbr == 0)
			manager->updateObjectProperties(*outProperty, DecelerationTime);
		outProperty->erasedZoneNbr++;
	}
	else
	{
		if (Cast<ACharacter>(OtherActor))
			return;
		AIdentityZoneManager::FErasedObjectProperties&	properties = manager->createNewProperties(OtherComp, DecelerationTime);
		properties.erasedZoneNbr = 1;
	}
}

void	UIdentityEraserComponent::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	bool isPropertyExisting = false;
	int id = 0;
	AIdentityZoneManager::FErasedObjectProperties* outProperty = manager->containsErasedObjectProperties(OtherComp, isPropertyExisting, id);
	if (isPropertyExisting)
	{
		if (outProperty->memoryZoneNbr != 0)
			outProperty->erasedZoneNbr--;
		else if (outProperty->erasedZoneNbr == 1)
		{
			if (outProperty->bWasSimulatingPhysics)
				OtherComp->SetSimulatePhysics(true);
			if (outProperty->chemicalComponent.IsValid())
				outProperty->chemicalComponent->GiveIdentity(outProperty->previousChemicalState);
			if (outProperty->interactableComponent.IsValid())
				outProperty->interactableComponent->GiveIdentity();
			manager->RemoveAffectedObject(id);
		}
		else
			outProperty->erasedZoneNbr--;
	}
}