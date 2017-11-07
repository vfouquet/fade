// Fill out your copyright notice in the Description page of Project Settings.

#include "ChemicalWaterComponent.h"

UChemicalWaterComponent::UChemicalWaterComponent()
{
	type = EChemicalType::Water;
}

void	UChemicalWaterComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

EChemicalTransformation		UChemicalWaterComponent::getEffectiveEffect(EChemicalType const& otherType, EChemicalState const& otherState) const
{
	return EChemicalTransformation::None;
}

EChemicalTransformation		UChemicalWaterComponent::getPotentialNextTransformation() const
{
	return EChemicalTransformation::None;
}

EChemicalState	UChemicalWaterComponent::getNextState(EChemicalTransformation const& transformation) const
{
	return EChemicalState::None;
}