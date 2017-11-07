// Fill out your copyright notice in the Description page of Project Settings.

#include "ChemicalRockComponent.h"

UChemicalRockComponent::UChemicalRockComponent()
{
	type = EChemicalType::Rock;
}

void	UChemicalRockComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	/*
	if (state == EChemicalState::None)
	{
	UE_LOG(LogTemp, Warning, TEXT("Basic rock"));
	}
	else if (state == EChemicalState::Oiled)
	{
	UE_LOG(LogTemp, Warning, TEXT("Oiled rock"));
	}
	else if (state == EChemicalState::Fire)
	{
	UE_LOG(LogTemp, Warning, TEXT("Fired rock"));
	}
	*/
}

EChemicalTransformation		UChemicalRockComponent::getEffectiveEffect(EChemicalType const& otherType, EChemicalState const& otherState) const
{
	if (state == EChemicalState::None)
	{
		if (otherType == EChemicalType::Oil && otherState == EChemicalState::None)
			return EChemicalTransformation::Oiling;
	}
	else if (state == EChemicalState::Oiled)
	{
		if (otherType == EChemicalType::Wood || otherType == EChemicalType::Rock)
		{
			if (otherState == EChemicalState::Fire)
				return EChemicalTransformation::Burning;
		}
		else if (otherType == EChemicalType::Fire)
			return EChemicalTransformation::Burning;
	}
	return EChemicalTransformation::None;
}

EChemicalTransformation		UChemicalRockComponent::getPotentialNextTransformation() const
{
	return EChemicalTransformation::None;
}

EChemicalState	UChemicalRockComponent::getNextState(EChemicalTransformation const& transformation) const
{
	if (transformation == EChemicalTransformation::Burning)
		return EChemicalState::None;
	else if (transformation == EChemicalTransformation::Drowning)
		return EChemicalState::Wet;
	else if (transformation == EChemicalTransformation::Oiling)
		return EChemicalState::Oiled;
	return EChemicalState::None;
}