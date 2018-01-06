// Fill out your copyright notice in the Description page of Project Settings.

#include "ChemicalCeramicComponent.h"
#include "Components/PrimitiveComponent.h"

UChemicalCeramicComponent::UChemicalCeramicComponent()
{
	type = EChemicalType::Ceramic;
}

EChemicalTransformation		UChemicalCeramicComponent::getEffectiveEffect(EChemicalType const& otherType, EChemicalState const& otherState) const
{
	if (state == EChemicalState::None)
	{
		if (otherType == EChemicalType::Oil && otherState == EChemicalState::None)
			return EChemicalTransformation::Staining;
	}
	else if (state == EChemicalState::Stained)
	{
		if (otherType == EChemicalType::Wood || otherType == EChemicalType::Ceramic)
		{
			if (otherState == EChemicalState::Burning)
				return EChemicalTransformation::Burning;
		}
		else if (otherType == EChemicalType::Fire)
			return EChemicalTransformation::Burning;
	}
	return EChemicalTransformation::None;
}

EChemicalTransformation		UChemicalCeramicComponent::getPotentialSelfNextTransformation() const
{
	return EChemicalTransformation::None;
}

EChemicalState	UChemicalCeramicComponent::getNextState(EChemicalTransformation const& transformation) const
{
	if (transformation == EChemicalTransformation::Burning)
		return EChemicalState::None;
	else if (transformation == EChemicalTransformation::Drenching)
		return EChemicalState::Drenched;
	else if (transformation == EChemicalTransformation::Staining)
		return EChemicalState::Stained;
	return EChemicalState::None;
}

bool	UChemicalCeramicComponent::computePercussionBreakability(UPrimitiveComponent* other)
{
	UPrimitiveComponent*	primitive = Cast<UPrimitiveComponent>(AssociatedComponent.GetComponent(GetOwner()));	
	UChemicalComponent*	otherComp = findAssociatedChemicalComponent(other);
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