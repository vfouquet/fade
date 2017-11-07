// Fill out your copyright notice in the Description page of Project Settings.

#include "ChemicalFireComponent.h"

UChemicalFireComponent::UChemicalFireComponent()
{
	type = EChemicalType::Fire;
}

void	UChemicalFireComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

EChemicalTransformation		UChemicalFireComponent::getEffectiveEffect(EChemicalType const& otherType, EChemicalState const& otherState) const
{
	if (otherType == EChemicalType::Water)
	{
		if (otherState == EChemicalState::None)
			return state == EChemicalState::Extinguished ? EChemicalTransformation::None : EChemicalTransformation::Drowning;
	}
	return EChemicalTransformation::None;
}

EChemicalTransformation		UChemicalFireComponent::getPotentialNextTransformation() const
{
	return EChemicalTransformation::None;
}

EChemicalState	UChemicalFireComponent::getNextState(EChemicalTransformation const& transformation) const
{
	if (transformation == EChemicalTransformation::Drowning)
	{
		if (state == EChemicalState::None)
			return EChemicalState::Extinguished;
	}
	return EChemicalState::None;
}