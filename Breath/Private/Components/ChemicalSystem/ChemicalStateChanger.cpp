// Fill out your copyright notice in the Description page of Project Settings.

#include "ChemicalStateChanger.h"

ChemicalStateChanger::ChemicalStateChanger(float value, bool needActorToUpdate)
{
	targetTime = value;
	actorNeeded = needActorToUpdate;
}

bool	ChemicalStateChanger::Update(float deltaTime)
{
	UE_LOG(LogTemp, Warning, TEXT("%f"), currentTime);
	if (actorNeeded && impactingActors.Num() == 0)
		return false;
	currentTime += deltaTime;
	return currentTime >= targetTime;
}

void	ChemicalStateChanger::AddImpactingActor(AActor* actor)
{
	if (impactingActors.Contains(actor))
		return;
	impactingActors.Add(actor);
}

void	ChemicalStateChanger::RemoveImpactingActor(AActor* actor)
{
	impactingActors.Remove(actor);
}