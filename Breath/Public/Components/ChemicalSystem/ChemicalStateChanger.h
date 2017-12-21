// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Containers/Array.h"

class AActor;

/**
 * 
 */
class BREATH_API ChemicalStateChanger
{
public:
	ChemicalStateChanger(float value, bool needActorToUpdate = true);
	~ChemicalStateChanger() = default;

	bool	Update(float deltaTime);

	void	AddImpactingActor(AActor* actor);
	void	RemoveImpactingActor(AActor* actor);

	int32	GetImpactingActorsNumber() const { return impactingActors.Num(); }

private:
	TArray<AActor*>	impactingActors;
	float			targetTime = 0.0f;
	float			currentTime = 0.0f;
	bool			actorNeeded = true;
};
