// Fill out your copyright notice in the Description page of Project Settings.

#include "ChemicalComponent.h"

#include "Components/PrimitiveComponent.h"
#include "Components/DestructibleComponent.h"
#include "Engine/DestructibleMesh.h"

float	UChemicalComponent::WeightThresholdValue = 30.0f;
float	UChemicalComponent::SpeedThresholdValue = 50.0f;

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
	if (!owner) return;

	UPrimitiveComponent* tempPrimitive = Cast<UPrimitiveComponent>(AssociatedComponent.GetComponent(owner));
	if (!tempPrimitive) return;
	
	FScriptDelegate	beginOverlapDel;
	beginOverlapDel.BindUFunction(this, "OnOverlap");
	tempPrimitive->OnComponentBeginOverlap.Add(beginOverlapDel);
	FScriptDelegate	endOverlapDel;
	endOverlapDel.BindUFunction(this, "OnEndOverlap");
	tempPrimitive->OnComponentEndOverlap.Add(endOverlapDel);
	FScriptDelegate	hitOverlap;
	hitOverlap.BindUFunction(this, "OnHit");
	tempPrimitive->OnComponentHit.Add(hitOverlap);
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
			EChemicalTransformation	previousTransformation = stateChanger.Key;
			currentChangers.Empty();
			notifyChemicalStateChanged(previousTransformation, previousState, state);
			break;
		}
	}
}

void	UChemicalComponent::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	UChemicalComponent*	comp = findAssociatedChemicalComponent(OtherComp);// OtherActor->FindComponentByClass<UChemicalComponent>();
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

void	UChemicalComponent::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UChemicalComponent*	comp = findAssociatedChemicalComponent(OtherComp);	//OtherActor->FindComponentByClass<UChemicalComponent>();
	if (!comp)
		return;

	EChemicalTransformation transformation = getEffectiveEffect(comp->GetType(), comp->GetState());
	if (transformation != EChemicalTransformation::None)
	{
		if (currentChangers.Contains(transformation))
		{
			ChemicalStateChanger&	transformationStateChanger = currentChangers[transformation];
			transformationStateChanger.RemoveImpactingActor(OtherActor);
			if (transformationStateChanger.GetImpactingActorsNumber() == 0 && !bPersistantTransformation)
				currentChangers.Remove(transformation);
		}
	}
}
	
void	UChemicalComponent::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (computePercussionBreakability(OtherComp))
	{
		UE_LOG(LogTemp, Warning, TEXT("Break"));
		applyChemicalPhysics();
	}
}

void	UChemicalComponent::EraseIdentity()
{
	currentChangers.Empty();
	stateChangedDelegate.Broadcast(EChemicalTransformation::Erasing, state, EChemicalState::NoIdentity);
	state = EChemicalState::NoIdentity;
}

void	UChemicalComponent::GiveIdentity()
{
	state = EChemicalState::None;
	stateChangedDelegate.Broadcast(EChemicalTransformation::GivingIdentity, EChemicalState::NoIdentity, EChemicalState::None);
}

ChemicalStateChanger&	UChemicalComponent::addStateChanger(EChemicalTransformation transformation)
{
	if (transformation == EChemicalTransformation::Drenching)
	{
		for (auto& stateChanger : currentChangers)
		{
			if (stateChanger.Key == EChemicalTransformation::Burning || stateChanger.Key == EChemicalTransformation::Staining)
				currentChangers.Remove(stateChanger.Key);
		}
	}
	ChemicalStateChanger	temp(2.0f);
	currentChangers.Add(transformation, temp);
	return currentChangers[transformation];
}
	
UChemicalComponent*	UChemicalComponent::findAssociatedChemicalComponent(UPrimitiveComponent* referenceComponent)
{
	AActor* refCompOwner = referenceComponent->GetOwner();
	if (!refCompOwner)	return nullptr;

	TArray<UActorComponent*>	chemicalComponents = refCompOwner->GetComponentsByClass(UChemicalComponent::StaticClass());
	for (auto& actorComp : chemicalComponents)
	{
		UChemicalComponent*	chemicalComp = Cast<UChemicalComponent>(actorComp);
		if (!chemicalComp) continue;
		UPrimitiveComponent* tempPrimitive = Cast<UPrimitiveComponent>(chemicalComp->AssociatedComponent.GetComponent(refCompOwner));
		if (tempPrimitive == referenceComponent)
			return chemicalComp;
	}
	return nullptr;
}

void	UChemicalComponent::notifyChemicalStateChanged(EChemicalTransformation previousTransformation, EChemicalState previous, EChemicalState next)
{
	stateChangedDelegate.Broadcast(previousTransformation, previous, next);

	EChemicalTransformation	transformation = getPotentialSelfNextTransformation();
	if (transformation != EChemicalTransformation::None)
	{
		ChemicalStateChanger& stateChanger = addStateChanger(transformation);
		stateChanger.AddImpactingActor(GetOwner());
	}

	TArray<AActor*>	overlappingChemicalActors;
	GetOwner()->GetOverlappingActors(overlappingChemicalActors);
	for (auto It = overlappingChemicalActors.CreateConstIterator(); It; ++It)
	{
		UChemicalComponent*	comp = (*It)->FindComponentByClass<UChemicalComponent>();
		if (!comp)
			continue;
		EChemicalTransformation transformation = getEffectiveEffect(comp->GetType(), comp->GetState());
		if (transformation == EChemicalTransformation::None)
			continue;
		if (currentChangers.Contains(transformation))
			currentChangers[transformation].AddImpactingActor(comp->GetOwner());
		else
		{
			ChemicalStateChanger& stateChanger = addStateChanger(transformation);
			stateChanger.AddImpactingActor(comp->GetOwner());
		}
	}
}

void	UChemicalComponent::applyChemicalPhysics()
{
	/*
	UDestructibleComponent*	destructibleComp = Cast<UDestructibleComponent>(AssociatedComponent.GetComponent(GetOwner()));
	if (destructibleComp)
		destructibleComp->ApplyRadiusDamage(destructibleComp->DestructibleMesh->DefaultDestructibleParameters.DamageParameters.DamageThreshold, destructibleComp->GetComponentLocation(), 0.0f, 0.0f, true);
	*/
	DestroyComponent();
}