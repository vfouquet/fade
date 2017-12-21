// Fill out your copyright notice in the Description page of Project Settings.

#include "ChemicalFireComponent.h"

UChemicalFireComponent::UChemicalFireComponent()
{
	type = EChemicalType::Fire;
}

EChemicalTransformation		UChemicalFireComponent::getEffectiveEffect(EChemicalType const& otherType, EChemicalState const& otherState) const
{
	return EChemicalTransformation::None;
}

EChemicalTransformation		UChemicalFireComponent::getPotentialSelfNextTransformation() const
{
	return EChemicalTransformation::None;
}

EChemicalState	UChemicalFireComponent::getNextState(EChemicalTransformation const& transformation) const
{
	return EChemicalState::None;
}