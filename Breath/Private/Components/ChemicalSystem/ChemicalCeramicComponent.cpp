// Fill out your copyright notice in the Description page of Project Settings.

#include "ChemicalCeramicComponent.h"
#include "Components/PrimitiveComponent.h"

void UChemicalCeramicComponent::InitializeComponent()
{
	Super::InitializeComponent();
	type = EChemicalType::Ceramic;
}

ChemicalStateChanger&	UChemicalCeramicComponent::addStateChanger(EChemicalTransformation transformation)
{
	float time = 0.0f;
	if (transformation == EChemicalTransformation::Burning)
	{
		if (state == EChemicalState::None)
			time = BreakByFireTime;
	}
	ChemicalStateChanger	temp(time);
	currentChangers.Add(transformation, temp);
	return currentChangers[transformation];
}

EChemicalTransformation		UChemicalCeramicComponent::getEffectiveEffect(EChemicalType const& otherType, EChemicalState const& otherState) const
{
	if (otherType == EChemicalType::Fire && otherState == EChemicalState::None)
		return EChemicalTransformation::Burning;
	else if ((otherType == EChemicalType::Rock || otherType == EChemicalType::Wood) && otherState == EChemicalState::Burning)
		return EChemicalTransformation::Burning;
	return EChemicalTransformation::None;
}

EChemicalState	UChemicalCeramicComponent::getNextState(EChemicalTransformation const& transformation) const
{
	if (transformation == EChemicalTransformation::Burning)
	{
		if (state == EChemicalState::None)
			return EChemicalState::Extinguished;
	}
	return EChemicalState::None;
}

bool	UChemicalCeramicComponent::computePercussionBreakability(UPrimitiveComponent* other)
{
	UChemicalComponent*	otherComp = FindAssociatedChemicalComponent(other);
	if (!otherComp || !associatedComponent)
	{

		return false;
	}
	if (associatedComponent->GetComponentVelocity().Size() > SpeedThresholdValue)
		return true;
	if (other->GetComponentVelocity().Size() < SpeedThresholdValue)
		return false;
	if (otherComp->GetType() == EChemicalType::Rock || otherComp->GetType() == EChemicalType::Wood)
		return true;
	return false;
}