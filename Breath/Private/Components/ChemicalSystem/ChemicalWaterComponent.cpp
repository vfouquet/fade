// Fill out your copyright notice in the Description page of Project Settings.

#include "ChemicalWaterComponent.h"

void UChemicalWaterComponent::InitializeComponent()
{
	Super::InitializeComponent();
	type = EChemicalType::Water;
	if (InsideBody)
		state = AlreadyFilled ? EChemicalState::Contained : EChemicalState::Extinguished;
}

void	UChemicalWaterComponent::SetOutsideBody()
{
	if (state == EChemicalState::Contained)
	{
		state = EChemicalState::None;
		stateChangedDelegate.Broadcast(EChemicalTransformation::None, EChemicalState::Contained, EChemicalState::None);
		refreshChangersWithCurrentInteractions();
	}
	InsideBody = false;
}

ChemicalStateChanger&	UChemicalWaterComponent::addStateChanger(EChemicalTransformation transformation)
{
	float time = 0.0f;
	if (transformation == EChemicalTransformation::Drenching)
		time = FillingTime;

	ChemicalStateChanger	temp(time);
	currentChangers.Add(transformation, temp);
	return currentChangers[transformation];
}

EChemicalTransformation		UChemicalWaterComponent::getEffectiveEffect(EChemicalType const& otherType, EChemicalState const& otherState) const
{
	if (state == EChemicalState::Extinguished)
	{
		if (InsideBody == false)
			return EChemicalTransformation::None;
		if (otherType == EChemicalType::Water && otherState == EChemicalState::None)
			return EChemicalTransformation::Drenching;
	}
	return EChemicalTransformation::None;
}

EChemicalState	UChemicalWaterComponent::getNextState(EChemicalTransformation const& transformation) const
{
	if (transformation == EChemicalTransformation::Drenching)
	{
		if (state == EChemicalState::Extinguished)
			return EChemicalState::Contained;
	}
	return EChemicalState::None;
}