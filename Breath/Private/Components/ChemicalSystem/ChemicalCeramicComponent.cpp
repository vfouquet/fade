// Fill out your copyright notice in the Description page of Project Settings.

#include "ChemicalCeramicComponent.h"
#include "Components/PrimitiveComponent.h"

void UChemicalCeramicComponent::InitializeComponent()
{
	Super::InitializeComponent();
	type = EChemicalType::Ceramic;
}

bool	UChemicalCeramicComponent::computePercussionBreakability(UPrimitiveComponent* other)
{
	UPrimitiveComponent*	primitive = Cast<UPrimitiveComponent>(AssociatedComponent.GetComponent(GetOwner()));	
	UChemicalComponent*	otherComp = FindAssociatedChemicalComponent(other);
	if (!otherComp)
	{

		return false;
	}
	if (primitive->GetComponentVelocity().Size() > SpeedThresholdValue)
		return true;
	if (other->GetComponentVelocity().Size() < SpeedThresholdValue)
		return false;
	if (otherComp->GetType() == EChemicalType::Rock || otherComp->GetType() == EChemicalType::Wood)
		return true;
	return false;
}