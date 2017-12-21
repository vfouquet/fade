// Fill out your copyright notice in the Description page of Project Settings.

#include "ChemicalOilComponent.h"

UChemicalOilComponent::UChemicalOilComponent()
{
	type = EChemicalType::Oil;
}

EChemicalTransformation		UChemicalOilComponent::getEffectiveEffect(EChemicalType const& otherType, EChemicalState const& otherState) const
{
	if (otherType == EChemicalType::Fire && otherState == EChemicalState::None)
		return EChemicalTransformation::Burning;
	return EChemicalTransformation::None;
}

EChemicalTransformation		UChemicalOilComponent::getPotentialSelfNextTransformation() const
{
	return EChemicalTransformation::None;
}

EChemicalState	UChemicalOilComponent::getNextState(EChemicalTransformation const& transformation) const
{
	if (transformation == EChemicalTransformation::Burning)
		return EChemicalState::Scorched;
	return EChemicalState::None;
}