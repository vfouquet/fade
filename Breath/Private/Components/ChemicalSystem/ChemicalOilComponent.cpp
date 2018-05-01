// Fill out your copyright notice in the Description page of Project Settings.

#include "ChemicalOilComponent.h"

void UChemicalOilComponent::InitializeComponent()
{
	Super::InitializeComponent();
	type = EChemicalType::Oil;
}

ChemicalStateChanger&	UChemicalOilComponent::addStateChanger(EChemicalTransformation transformation)
{
	float	time = 0.0f;
	if (transformation == EChemicalTransformation::Burning)
	{
		if (state == EChemicalState::None)
			time = noneToLit;
		else if (state == EChemicalState::Lit)
			time = litToBurning;
		else if (state == EChemicalState::Burning)
			time = burningToDestroy;
	}
	else if (transformation == EChemicalTransformation::Drenching)
		time = toDestroy;
	ChemicalStateChanger	temp(time);
	currentChangers.Add(transformation, temp);
	return currentChangers[transformation];
}

void	UChemicalOilComponent::getStateChangedUselessTransformation(TArray<EChemicalTransformation>& returnValues, EChemicalTransformation previousTransformation) const
{
	if (previousTransformation == EChemicalTransformation::Drenching)
		returnValues.Add(EChemicalTransformation::Burning);
}

EChemicalTransformation		UChemicalOilComponent::getEffectiveEffect(EChemicalType const& otherType, EChemicalState const& otherState) const
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
	else if (otherType == EChemicalType::Water && (otherState == EChemicalState::None || otherState == EChemicalState::Splashing))
	{
		if (canBeDrenched())
			return EChemicalTransformation::Drenching;
	}
	return EChemicalTransformation::None;
}

EChemicalTransformation		UChemicalOilComponent::getPotentialSelfNextTransformation() const
{
	if (state == EChemicalState::Burning)
		return EChemicalTransformation::Burning;
	return EChemicalTransformation::None;
}

EChemicalState	UChemicalOilComponent::getNextState(EChemicalTransformation const& transformation) const
{
	if (transformation == EChemicalTransformation::Burning)
	{
		if (state == EChemicalState::None)
			return EChemicalState::Lit;
		else if (state == EChemicalState::Lit)
			return EChemicalState::Burning;
		else if (state == EChemicalState::Burning)
			return EChemicalState::Scorched;
	}
	else if (transformation == EChemicalTransformation::Drenching)
		return EChemicalState::Drenched;
	return EChemicalState::None;
}