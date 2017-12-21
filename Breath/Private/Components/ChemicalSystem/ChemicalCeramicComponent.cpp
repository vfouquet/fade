// Fill out your copyright notice in the Description page of Project Settings.

#include "ChemicalCeramicComponent.h"

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


