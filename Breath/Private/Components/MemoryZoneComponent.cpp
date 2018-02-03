// Fill out your copyright notice in the Description page of Project Settings.

#include "MemoryZoneComponent.h"

#include "GameFramework/Character.h"

void	UMemoryZoneComponent::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	int id = 0;
	bool isPropertyExisting = false;
	FErasedObjectProperties* outProperty = containsErasedObjectProperties(OtherComp, isPropertyExisting, id);
	if (isPropertyExisting)
	{
		if (outProperty->memoryZoneNbr == 0 && outProperty->erasedZoneNbr > 0)
		{
			if (outProperty->bWasSimulatingPhysics)
				OtherComp->SetSimulatePhysics(true);
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
		auto& properties = createNewProperties(OtherComp);
		properties.bDecelerating = false;
		properties.memoryZoneNbr++;
	}
}

void	UMemoryZoneComponent::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	int id = 0;
	bool isPropertyExisting = false;
	FErasedObjectProperties* outProperty = containsErasedObjectProperties(OtherComp, isPropertyExisting, id);
	if (isPropertyExisting)
	{
		if (outProperty->memoryZoneNbr == 1)
		{
			if (outProperty->erasedZoneNbr > 0)
			{
				updateObjectProperties(*outProperty);
				outProperty->memoryZoneNbr = 0;
			}
			else
				affectedObjects.RemoveAt(id);
		}
		else
			outProperty->memoryZoneNbr--;
	}
}