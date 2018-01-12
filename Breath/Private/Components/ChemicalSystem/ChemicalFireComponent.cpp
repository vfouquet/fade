// Fill out your copyright notice in the Description page of Project Settings.

#include "ChemicalFireComponent.h"

UChemicalFireComponent::UChemicalFireComponent()
{
	type = EChemicalType::Fire;
	state = bExtinguishedAtLaunch ? EChemicalState::Extinguished : EChemicalState::None;
}

ChemicalStateChanger&	UChemicalFireComponent::addStateChanger(EChemicalTransformation transformation)
{
	float time = 0.0f;
	if (transformation == EChemicalTransformation::Drenching)
		time = toExtinguished;
	else if (transformation == EChemicalTransformation::Burning)
		time = extinguishedToBurning;

	ChemicalStateChanger	temp(time);
	currentChangers.Add(transformation, temp);
	return currentChangers[transformation];
}

EChemicalTransformation		UChemicalFireComponent::getEffectiveEffect(EChemicalType const& otherType, EChemicalState const& otherState) const
{
	if (otherType == EChemicalType::Water && otherState == EChemicalState::None)
	{
		if (canBeDrenched())
			return EChemicalTransformation::Drenching;
	}
	else if (otherType == EChemicalType::Fire && otherState == EChemicalState::None)
	{
		if (canBurn())
			return EChemicalTransformation::Burning;
	}
	else if ((otherType == EChemicalType::Rock || otherType == EChemicalType::Wood) && otherState == EChemicalState::Burning)
	{
		if (canBurn())
			return EChemicalTransformation::Burning;
	}
	return EChemicalTransformation::None;
}

EChemicalTransformation		UChemicalFireComponent::getPotentialSelfNextTransformation() const
{
	return EChemicalTransformation::None;
}

EChemicalState	UChemicalFireComponent::getNextState(EChemicalTransformation const& transformation) const
{
	if (transformation == EChemicalTransformation::Drenching)
	{
		if (state == EChemicalState::None)
			return EChemicalState::Extinguished;
	}
	else if (transformation == EChemicalTransformation::Burning)
	{
		if (state == EChemicalState::Extinguished)
			return EChemicalState::None;
	}
	return EChemicalState::None;
}