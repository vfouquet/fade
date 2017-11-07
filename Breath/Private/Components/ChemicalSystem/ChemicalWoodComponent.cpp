// Fill out your copyright notice in the Description page of Project Settings.

#include "ChemicalWoodComponent.h"

UChemicalWoodComponent::UChemicalWoodComponent()
{
	type = EChemicalType::Wood;
}

void UChemicalWoodComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	/*
	if (state == EChemicalState::None)
	{
	UE_LOG(LogTemp, Warning, TEXT("Normal box"));
	}
	else if (state == EChemicalState::Fire)
	{
	UE_LOG(LogTemp, Warning, TEXT("This box is on fire"));
	}
	else if (state == EChemicalState::Ashes)
	{
	UE_LOG(LogTemp, Warning, TEXT("This box turned into ashes"));
	}
	else if (state == EChemicalState::Wet)
	{
	UE_LOG(LogTemp, Warning, TEXT("This box is Wet"));
	}
	else if (state == EChemicalState::Oiled)
	{
	UE_LOG(LogTemp, Warning, TEXT("This box is oiled"));
	}
	*/
}

EChemicalTransformation		UChemicalWoodComponent::getEffectiveEffect(EChemicalType const& otherType, EChemicalState const& otherState) const
{
	if (otherType == EChemicalType::Fire && otherState == EChemicalState::None)
	{
		if (state == EChemicalState::None || state == EChemicalState::Fire
			|| state == EChemicalState::Oiled)
			return EChemicalTransformation::Burning;
	}
	else if (otherType == EChemicalType::Wood && otherState == EChemicalState::Fire)
	{
		if (state == EChemicalState::None || state == EChemicalState::Fire
			|| state == EChemicalState::Oiled)
			return EChemicalTransformation::Burning;
	}
	else if (otherType == EChemicalType::Water)
	{
		if (state == EChemicalState::None || state == EChemicalState::Fire || state == EChemicalState::Oiled)
			return EChemicalTransformation::Drowning;
	}
	else if (otherType == EChemicalType::Oil && otherState == EChemicalState::None)
		return EChemicalTransformation::Oiling;
	return EChemicalTransformation::None;
}

EChemicalTransformation		UChemicalWoodComponent::getPotentialNextTransformation() const
{
	if (state == EChemicalState::Fire)
		return EChemicalTransformation::Burning;
	return EChemicalTransformation::None;
}

EChemicalState	UChemicalWoodComponent::getNextState(EChemicalTransformation const& transformation) const
{
	if (transformation == EChemicalTransformation::Burning)
	{
		if (state == EChemicalState::None)
			return EChemicalState::Fire;
		else if (state == EChemicalState::Fire)
			return EChemicalState::Ashes;
		else if (state == EChemicalState::Oiled)
			return EChemicalState::Fire;
	}
	else if (transformation == EChemicalTransformation::Drowning)
	{
		if (state == EChemicalState::None || state == EChemicalState::Fire)
			return EChemicalState::Wet;
	}
	else if (transformation == EChemicalTransformation::Oiling)
		return EChemicalState::Oiled;
	return EChemicalState::None;
}