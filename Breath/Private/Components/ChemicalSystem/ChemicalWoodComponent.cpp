// Fill out your copyright notice in the Description page of Project Settings.

#include "ChemicalWoodComponent.h"

#include "Components/PrimitiveComponent.h"

UChemicalWoodComponent::UChemicalWoodComponent()
{
	type = EChemicalType::Wood;
}

ChemicalStateChanger&	UChemicalWoodComponent::addStateChanger(EChemicalTransformation transformation)
{
	float time = 0.0f;
	if (transformation == EChemicalTransformation::Drenching)
	{
		for (auto& stateChanger : currentChangers)
		{
			if (stateChanger.Key == EChemicalTransformation::Burning || stateChanger.Key == EChemicalTransformation::Staining)
				currentChangers.Remove(stateChanger.Key);
		}
		time = toDrench;
	}
	else if (transformation == EChemicalTransformation::Burning)
	{
		if (state == EChemicalState::Drenched)
			time = drenchToNormal;
		else if (state == EChemicalState::None)
			time = normalToLit;
		else if (state == EChemicalState::Lit)
			time = litToBurning;
		else if (state == EChemicalState::Burning)
			time = burningToScorched;
	}
	ChemicalStateChanger	temp(time);
	currentChangers.Add(transformation, temp);
	return currentChangers[transformation];
}

EChemicalTransformation		UChemicalWoodComponent::getEffectiveEffect(EChemicalType const& otherType, EChemicalState const& otherState) const
{
	if (otherType == EChemicalType::Fire && otherState == EChemicalState::None)
	{
		if (canBurn())
			return EChemicalTransformation::Burning;
	}
	else if (otherType == EChemicalType::Wood && otherState == EChemicalState::Burning)
	{
		if (canBurn())
			return EChemicalTransformation::Burning;
	}
	else if (otherType == EChemicalType::Water)
	{
		if (state == EChemicalState::None || state == EChemicalState::Lit || state == EChemicalState::Burning || state == EChemicalState::Stained)
			return EChemicalTransformation::Drenching;
	}
	else if (otherType == EChemicalType::Oil && otherState == EChemicalState::None)
		return EChemicalTransformation::Staining;
	return EChemicalTransformation::None;
}

EChemicalTransformation		UChemicalWoodComponent::getPotentialSelfNextTransformation() const
{
	if (state == EChemicalState::Burning)
		return EChemicalTransformation::Burning;
	return EChemicalTransformation::None;
}

EChemicalState	UChemicalWoodComponent::getNextState(EChemicalTransformation const& transformation) const
{
	if (transformation == EChemicalTransformation::Burning)
	{
		if (state == EChemicalState::None)
			return EChemicalState::Lit;
		else if (state == EChemicalState::Lit)
			return EChemicalState::Burning;
		else if (state == EChemicalState::Burning)
			return EChemicalState::Scorched;
		else if (state == EChemicalState::Stained)
			return EChemicalState::Lit;
		else if (state == EChemicalState::Drenched)
			return EChemicalState::None;
	}
	else if (transformation == EChemicalTransformation::Drenching)
	{
		if (state == EChemicalState::None || state == EChemicalState::Lit || state == EChemicalState::Burning)
			return EChemicalState::Drenched;
	}
	else if (transformation == EChemicalTransformation::Staining && canBeStained())
		return EChemicalState::Stained;
	return EChemicalState::None;
}

bool	UChemicalWoodComponent::computePercussionBreakability(UPrimitiveComponent* other)
{
	if (other->GetComponentVelocity().Size() < SpeedThresholdValue)
		return false;
	UPrimitiveComponent*	primitive = Cast<UPrimitiveComponent>(AssociatedComponent.GetComponent(GetOwner()));
	float woodMass = primitive->GetMass();
	UChemicalComponent*	otherComp = findAssociatedChemicalComponent(other);
	if (!otherComp)
	{
		
		return false;
	}
	if (otherComp->GetType() == EChemicalType::Rock)
	{
		float	stoneMass = other->GetMass();
		UE_LOG(LogTemp, Warning, TEXT("Wood mass : %f      Rock mass : %f"), woodMass, stoneMass);
		if (stoneMass > (woodMass * 0.5f))
			return true;
		return false;
	}
	return false;
}