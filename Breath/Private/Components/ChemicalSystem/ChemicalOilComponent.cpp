// Fill out your copyright notice in the Description page of Project Settings.

#include "ChemicalOilComponent.h"

UChemicalOilComponent::UChemicalOilComponent()
{
	type = EChemicalType::Oil;
}

void	UChemicalOilComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

EChemicalTransformation		UChemicalOilComponent::getEffectiveEffect(EChemicalType const& otherType, EChemicalState const& otherState) const
{
	if (otherType == EChemicalType::Fire && otherState == EChemicalState::None)
		return EChemicalTransformation::Burning;
	return EChemicalTransformation::None;
}

EChemicalTransformation		UChemicalOilComponent::getPotentialNextTransformation() const
{
	return EChemicalTransformation::None;
}

EChemicalState	UChemicalOilComponent::getNextState(EChemicalTransformation const& transformation) const
{
	if (transformation == EChemicalTransformation::Burning)
		return EChemicalState::Ashes;
	return EChemicalState::None;
}