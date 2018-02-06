// Fill out your copyright notice in the Description page of Project Settings.

#include "ChemicalRockComponent.h"

void UChemicalRockComponent::InitializeComponent()
{
	Super::InitializeComponent();
	type = EChemicalType::Rock;
}

ChemicalStateChanger&	UChemicalRockComponent::addStateChanger(EChemicalTransformation transformation)
{
	float time = 0.0f;
	if (transformation == EChemicalTransformation::Burning)
	{
		if (state == EChemicalState::Stained)
			time = stainedToLit;
		else if (state == EChemicalState::Lit)
			time = litToBurning;
		else if (state == EChemicalState::Burning)
			time = burningToNone;
	}
	else if (transformation == EChemicalTransformation::Staining)
		time = toStained;
	else if (transformation == EChemicalTransformation::Drenching)
		time = toDrenched;
	ChemicalStateChanger	temp(time);
	currentChangers.Add(transformation, temp);
	return currentChangers[transformation];
}

void	UChemicalRockComponent::getStateChangedUselessTransformation(TArray<EChemicalTransformation>& returnValues, EChemicalTransformation previousTransformation) const
{
	if (previousTransformation == EChemicalTransformation::Drenching)
	{
		returnValues.Add(EChemicalTransformation::Burning);
		returnValues.Add(EChemicalTransformation::Staining);
	}
	else if (previousTransformation == EChemicalTransformation::Burning)
		returnValues.Add(EChemicalTransformation::Staining);
}

EChemicalTransformation		UChemicalRockComponent::getEffectiveEffect(EChemicalType const& otherType, EChemicalState const& otherState) const
{
	if (otherType == EChemicalType::Fire && otherState == EChemicalState::None)
	{
		if (canBurn())
			return EChemicalTransformation::Burning;
	}
	else if ((otherType == EChemicalType::Rock || otherType == EChemicalType::Wood) && otherState == EChemicalState::Burning)
	{
		if (canBurn())
			return EChemicalTransformation::Burning;
	}
	else if (otherType == EChemicalType::Water)
	{
		if (canBeDrenched())
			return EChemicalTransformation::Drenching;
	}
	else if (otherType == EChemicalType::Oil && otherState == EChemicalState::None)
	{
		if (canBeStained())
			return EChemicalTransformation::Staining;
	}
	return EChemicalTransformation::None;
}

EChemicalTransformation		UChemicalRockComponent::getPotentialSelfNextTransformation() const
{
	if (state == EChemicalState::Burning)
		return EChemicalTransformation::Burning;
	return EChemicalTransformation::None;
}

EChemicalState	UChemicalRockComponent::getNextState(EChemicalTransformation const& transformation) const
{
	if (transformation == EChemicalTransformation::Burning)
	{
		if (state == EChemicalState::Stained)
			return EChemicalState::Lit;
		else if (state == EChemicalState::Lit)
			return EChemicalState::Burning;
		else if (state == EChemicalState::Burning)
			return EChemicalState::None;
	}
	else if (transformation == EChemicalTransformation::Drenching)
		return EChemicalState::Drenched;
	else if (transformation == EChemicalTransformation::Staining)
		return EChemicalState::Stained;
	return EChemicalState::None;
}