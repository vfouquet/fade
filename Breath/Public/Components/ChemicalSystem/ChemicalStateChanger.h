// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Containers/Array.h"
#include "Containers/Map.h"

class UPrimitiveComponent;

/**
 * 
 */
class BREATH_API ChemicalStateChanger
{
public:
	ChemicalStateChanger(float value);
	~ChemicalStateChanger() = default;

	bool	Update(float deltaTime);
	bool	RemoveIfNeeded(UPrimitiveComponent* mainComp);

	void	AddImpactingComponent(UPrimitiveComponent* actor);
	void	RemoveImpactingComponent(UPrimitiveComponent* actor);

	int32	GetImpactingComponentsNumber() const { return impactingComponents.Num(); }

private:
	TArray<UPrimitiveComponent*>		impactingComponents;
	float								targetTime = 0.0f;
	float								currentTime = 0.0f;
};
