// Fill out your copyright notice in the Description page of Project Settings.

#include "ChemicalWaterComponent.h"

UChemicalWaterComponent::UChemicalWaterComponent()
{
	type = EChemicalType::Water;
}

EChemicalTransformation		UChemicalWaterComponent::getEffectiveEffect(EChemicalType const& otherType, EChemicalState const& otherState) const
{
	return EChemicalTransformation::None;
}

EChemicalTransformation		UChemicalWaterComponent::getPotentialSelfNextTransformation() const
{
	return EChemicalTransformation::None;
}

EChemicalState	UChemicalWaterComponent::getNextState(EChemicalTransformation const& transformation) const
{
	return EChemicalState::None;
}