// Fill out your copyright notice in the Description page of Project Settings.

#include "ChemicalComponent.h"
// Sets default values for this component's properties
UChemicalComponent::UChemicalComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UChemicalComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	AActor* owner = GetOwner();
	if (!owner)
		return;
	FScriptDelegate	beginOverlapDel;
	beginOverlapDel.BindUFunction(this, "OnActorOverlap");
	owner->OnActorBeginOverlap.Add(beginOverlapDel);
	FScriptDelegate	endOverlapDel;
	endOverlapDel.BindUFunction(this, "OnActorEndOverlap");
	owner->OnActorEndOverlap.Add(endOverlapDel);
}


// Called every frame
void UChemicalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
	for (auto& stateChanger : currentChangers)
	{
		if (stateChanger.Value.Update(DeltaTime))
		{
			EChemicalState previousState = state;
			state = getNextState(stateChanger.Key);
			currentChangers.Empty();
			notifyChemicalStateChanged(previousState, state);
			break;
		}
	}
}

void	UChemicalComponent::OnActorOverlap(UPrimitiveComponent* overlapComp, AActor* OtherActor, UPrimitiveComponent* otherComp)
{
	UChemicalComponent*	comp = OtherActor->FindComponentByClass<UChemicalComponent>();
	if (!comp)
		return;

	EChemicalTransformation transformation = getEffectiveEffect(comp->GetType(), comp->GetState());
	if (transformation != EChemicalTransformation::None)
	{
		if (!currentChangers.Contains(transformation))
		{
			ChemicalStateChanger& stateChanger = addStateChanger(transformation);
			stateChanger.AddImpactingActor(OtherActor);
		}
		else
			currentChangers[transformation].AddImpactingActor(OtherActor);
	}
}

void	UChemicalComponent::OnActorEndOverlap(UPrimitiveComponent* overlapComp, AActor* OtherActor, UPrimitiveComponent* otherComp)
{
	UChemicalComponent*	comp = OtherActor->FindComponentByClass<UChemicalComponent>();
	if (!comp)
		return;

	EChemicalTransformation transformation = getEffectiveEffect(comp->GetType(), comp->GetState());
	if (transformation != EChemicalTransformation::None)
	{
		if (currentChangers.Contains(transformation))
			currentChangers[transformation].RemoveImpactingActor(OtherActor);
	}
}

void	UChemicalComponent::notifyChemicalStateChanged(EChemicalState previous, EChemicalState next)
{
	stateChangedDelegate.Broadcast(previous, next);

	EChemicalTransformation	tempTransformation = getPotentialNextTransformation();
	if (tempTransformation != EChemicalTransformation::None)
	{
		ChemicalStateChanger& stateChanger = addStateChanger(tempTransformation);
		stateChanger.AddImpactingActor(this->GetOwner());

		TArray<AActor*>	overlappingChemicalActors;
		GetOwner()->GetOverlappingActors(overlappingChemicalActors);
		for (auto It = overlappingChemicalActors.CreateConstIterator(); It; ++It)
		{
			UChemicalComponent*	comp = (*It)->FindComponentByClass<UChemicalComponent>();
			if (!comp)
				continue;
			EChemicalTransformation transformation = getEffectiveEffect(comp->GetType(), comp->GetState());
			if (transformation != EChemicalTransformation::None && transformation == tempTransformation)
				currentChangers[transformation].AddImpactingActor(GetOwner());
		}
	}
}

ChemicalStateChanger&	UChemicalComponent::addStateChanger(EChemicalTransformation transformation)
{
	if (transformation == EChemicalTransformation::Drowning)
	{
		for (auto& stateChanger : currentChangers)
		{
			if (stateChanger.Key == EChemicalTransformation::Burning || stateChanger.Key == EChemicalTransformation::Oiling)
				currentChangers.Remove(stateChanger.Key);
		}
	}
	ChemicalStateChanger	temp(2.0f);
	currentChangers.Add(transformation, temp);
	return currentChangers[transformation];
}