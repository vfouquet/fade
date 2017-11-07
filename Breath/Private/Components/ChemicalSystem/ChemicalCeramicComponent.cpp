// Fill out your copyright notice in the Description page of Project Settings.

#include "ChemicalCeramicComponent.h"

UChemicalCeramicComponent::UChemicalCeramicComponent()
{
	type = EChemicalType::Ceramic;
}

void	UChemicalCeramicComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	/*
	if (state == EChemicalState::None)
	{
	UE_LOG(LogTemp, Warning, TEXT("Basic Ceramic"));
	}
	else if (state == EChemicalState::Oiled)
	{
	UE_LOG(LogTemp, Warning, TEXT("Oiled Ceramic"));
	}
	else if (state == EChemicalState::Fire)
	{
	UE_LOG(LogTemp, Warning, TEXT("Fired Ceramic"));
	}
	*/
}

EChemicalTransformation		UChemicalCeramicComponent::getEffectiveEffect(EChemicalType const& otherType, EChemicalState const& otherState) const
{
	if (state == EChemicalState::None)
	{
		if (otherType == EChemicalType::Oil && otherState == EChemicalState::None)
			return EChemicalTransformation::Oiling;
	}
	else if (state == EChemicalState::Oiled)
	{
		if (otherType == EChemicalType::Wood || otherType == EChemicalType::Ceramic)
		{
			if (otherState == EChemicalState::Fire)
				return EChemicalTransformation::Burning;
		}
		else if (otherType == EChemicalType::Fire)
			return EChemicalTransformation::Burning;
	}
	return EChemicalTransformation::None;
}

EChemicalTransformation		UChemicalCeramicComponent::getPotentialNextTransformation() const
{
	return EChemicalTransformation::None;
}

EChemicalState	UChemicalCeramicComponent::getNextState(EChemicalTransformation const& transformation) const
{
	if (transformation == EChemicalTransformation::Burning)
		return EChemicalState::None;
	else if (transformation == EChemicalTransformation::Drowning)
		return EChemicalState::Wet;
	else if (transformation == EChemicalTransformation::Oiling)
		return EChemicalState::Oiled;
	return EChemicalState::None;
}


