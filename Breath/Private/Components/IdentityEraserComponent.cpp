// Fill out your copyright notice in the Description page of Project Settings.

#include "IdentityEraserComponent.h"
#include "GameFramework/Character.h"

#include "ChemicalComponent.h"
#include "InteractableComponent.h"

void	UIdentityEraserComponent::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	FErasedObjectProperties outProperty;
	int id = 0;
	if (containsErasedObjectProperties(OtherComp, outProperty, id))
		outProperty.erasedZoneNbr++;
	else
	{
		if (Cast<ACharacter>(OtherActor))
			return;
		FErasedObjectProperties	properties;
		properties.primitiveComponent = OtherComp;
		properties.bWasSimulatingPhysics = OtherComp->IsSimulatingPhysics();
		if (properties.bWasSimulatingPhysics)
		{
			properties.initialVelocity = OtherComp->GetComponentVelocity();
			properties.bDecelerating = true;
		}

		UChemicalComponent*	chemicalComp = UChemicalComponent::FindAssociatedChemicalComponent(OtherComp);
		if (chemicalComp)
		{
			properties.previousChemicalState = chemicalComp->GetState();
			chemicalComp->EraseIdentity();
			properties.chemicalComponent = chemicalComp;
		}
		UInteractableComponent* interactableComp = UInteractableComponent::FindAssociatedInteractableComponent(OtherComp);
		if (interactableComp)
		{
			properties.interactableComponent = interactableComp;
			interactableComp->EraseIdentity();
		}
		properties.erasedZoneNbr = 1;
		affectedObjects.Add(properties);
	}
}

void	UIdentityEraserComponent::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	FErasedObjectProperties outProperty;
	int id = 0;
	if (containsErasedObjectProperties(OtherComp, outProperty, id))
	{
		if (outProperty.memoryZoneNbr != 0)
			outProperty.erasedZoneNbr--;
		else if (outProperty.erasedZoneNbr == 1)
		{
			if (outProperty.bWasSimulatingPhysics)
				OtherComp->SetSimulatePhysics(true);
			if (outProperty.chemicalComponent.IsValid())
				outProperty.chemicalComponent->GiveIdentity(outProperty.previousChemicalState);
			if (outProperty.interactableComponent.IsValid())
				outProperty.interactableComponent->GiveIdentity();
			affectedObjects.RemoveAt(id);
		}
		else
			outProperty.erasedZoneNbr--;
	}
}