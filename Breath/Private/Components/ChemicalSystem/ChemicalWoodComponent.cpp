// Fill out your copyright notice in the Description page of Project Settings.

#include "ChemicalWoodComponent.h"

#include "Components/PrimitiveComponent.h"

void UChemicalWoodComponent::InitializeComponent()
{
	Super::InitializeComponent();
	type = EChemicalType::Wood;
}

ChemicalStateChanger&	UChemicalWoodComponent::addStateChanger(EChemicalTransformation transformation)
{
	float time = 0.0f;
	if (transformation == EChemicalTransformation::Drenching)
		time = toDrench;
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
	else if (transformation == EChemicalTransformation::Staining)
		time = toStained;
	ChemicalStateChanger	temp(time);
	currentChangers.Add(transformation, temp);
	return currentChangers[transformation];
}
	
void	UChemicalWoodComponent::getStateChangedUselessTransformation(TArray<EChemicalTransformation>& returnValues, EChemicalTransformation previousTransformation) const
{
	if (previousTransformation == EChemicalTransformation::Drenching)
	{
		returnValues.Add(EChemicalTransformation::Burning);
		returnValues.Add(EChemicalTransformation::Staining);
	}
	else if (previousTransformation == EChemicalTransformation::Burning)
		returnValues.Add(EChemicalTransformation::Staining);
}

EChemicalTransformation		UChemicalWoodComponent::getEffectiveEffect(EChemicalType const& otherType, EChemicalState const& otherState) const
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
	else if (otherType == EChemicalType::Water && otherState == EChemicalState::None)
	{
		if (canBeDrenched())
			return EChemicalTransformation::Drenching;
	}
	else if (otherType == EChemicalType::Oil && otherState == EChemicalState::None)
	{
		if (canBeStained())
			return EChemicalTransformation::Staining;
	}
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
		if (state == EChemicalState::None || state == EChemicalState::Lit || state == EChemicalState::Burning || state == EChemicalState::Stained)
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
	UChemicalComponent*	otherComp = FindAssociatedChemicalComponent(other);
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