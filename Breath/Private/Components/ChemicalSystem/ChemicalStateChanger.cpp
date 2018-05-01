// Fill out your copyright notice in the Description page of Project Settings.

#include "ChemicalStateChanger.h"

#include "Components/PrimitiveComponent.h"

ChemicalStateChanger::ChemicalStateChanger(float value)
{
	targetTime = value;
}

bool	ChemicalStateChanger::Update(float deltaTime)
{
	//CHECK IF ACTOR EXIST
	impactingComponents.RemoveAll([&](TWeakObjectPtr<UPrimitiveComponent> item) { return !item.IsValid(); });

	if (impactingComponents.Num() == 0)
		return false;
	currentTime += deltaTime;
	return currentTime >= targetTime;
}
	
bool	ChemicalStateChanger::RemoveIfNeeded(UPrimitiveComponent* mainComp)
{
	impactingComponents.Remove(mainComp);
	return (impactingComponents.Num() == 0);
}

void	ChemicalStateChanger::AddImpactingComponent(UPrimitiveComponent* component)
{
	if (impactingComponents.Contains(component))
		return;
	impactingComponents.Add(component);
}

bool	ChemicalStateChanger::Contains(UPrimitiveComponent* prim) const
{
	return impactingComponents.ContainsByPredicate([&](TWeakObjectPtr<UPrimitiveComponent> PrimValue)
	{
		if (PrimValue.IsValid())
			return PrimValue.Get() == prim;
		return false;
	});
}