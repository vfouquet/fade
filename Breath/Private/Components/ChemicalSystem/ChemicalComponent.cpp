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

	associatedComponent = Cast<UPrimitiveComponent>(AssociatedComponent.GetComponent(owner));
	if (!associatedComponent) return;
	
	FScriptDelegate	beginOverlapDel;
	beginOverlapDel.BindUFunction(this, "OnOverlap");
	associatedComponent->OnComponentBeginOverlap.Add(beginOverlapDel);
	FScriptDelegate	endOverlapDel;
	endOverlapDel.BindUFunction(this, "OnEndOverlap");
	associatedComponent->OnComponentEndOverlap.Add(endOverlapDel);
	FScriptDelegate	hitOverlap;
	hitOverlap.BindUFunction(this, "OnHit");
	associatedComponent->OnComponentHit.Add(hitOverlap);
}


// Called every frame
void UChemicalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
	for (auto& stateChanger : currentChangers)
	{
		for (auto& hitComp : hitChemicalComponents)
		{
			float distance = FVector::Distance(associatedComponent->GetComponentLocation(), hitComp.Key->GetAssociatedComponent()->GetComponentLocation());
			if (distance > hitComp.Value + 25.0f)
			{
				hitChemicalComponents.Remove(hitComp.Key);
				if (stateChanger.Value.RemoveIfNeeded(hitComp.Key->GetAssociatedComponent()) && !bPersistantTransformation)
				{
					currentChangers.Remove(stateChanger.Key);
					continue;
				}
			}
		}
		if (stateChanger.Value.Update(DeltaTime))
		{
			EChemicalState previousState = state;
			state = getNextState(stateChanger.Key);
			EChemicalTransformation	previousTransformation = stateChanger.Key;
			TArray<EChemicalTransformation>	transformationToRemove;
			getStateChangedUselessTransformation(transformationToRemove, previousTransformation);
			currentChangers.Remove(previousTransformation);
			for (auto& transformation : transformationToRemove)
			{
				if (currentChangers.Contains(transformation))
					currentChangers.Remove(transformation);
			}
			notifyChemicalStateChanged(previousTransformation, previousState, state);
			break;
		}
	}
}

void	UChemicalComponent::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	UChemicalComponent*	comp = FindAssociatedChemicalComponent(OtherComp);
	if (!comp)
		return;

	EChemicalTransformation transformation = getEffectiveEffect(comp->GetType(), comp->GetState());
	if (transformation != EChemicalTransformation::None)
		addComponentToChangers(transformation, OtherComp);
}

void	UChemicalComponent::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UChemicalComponent*	comp = FindAssociatedChemicalComponent(OtherComp);
	if (!comp)
		return;

	EChemicalTransformation transformation = getEffectiveEffect(comp->GetType(), comp->GetState());
	if (transformation != EChemicalTransformation::None)
		removeComponentFromChangers(transformation, OtherComp);
}
	
void	UChemicalComponent::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (computePercussionBreakability(OtherComp))
	{
		UE_LOG(LogTemp, Warning, TEXT("Break"));
		applyChemicalPhysics();
	}
	else
	{
		UChemicalComponent*	chemicalComp = FindAssociatedChemicalComponent(OtherComp);
		if (!chemicalComp)
			return;
		EChemicalTransformation transformation = getEffectiveEffect(chemicalComp->GetType(), chemicalComp->GetState());
		if (transformation != EChemicalTransformation::None)
		{
			hitChemicalComponents.Add(chemicalComp, FVector::Distance(HitComponent->GetComponentLocation(), OtherComp->GetComponentLocation()));
			addComponentToChangers(transformation, OtherComp);
		}
	}
}

void	UChemicalComponent::EraseIdentity()
{
	currentChangers.Empty();
	stateChangedDelegate.Broadcast(EChemicalTransformation::Erasing, state, EChemicalState::NoIdentity);
}

void	UChemicalComponent::GiveIdentity(EChemicalState previousState)
{
	if (state != EChemicalState::NoIdentity)
		return;
	state = previousState;
	stateChangedDelegate.Broadcast(EChemicalTransformation::GivingIdentity, EChemicalState::NoIdentity, previousState);
	EChemicalTransformation	transformation = getPotentialSelfNextTransformation();
	if (transformation != EChemicalTransformation::None)
	{
		ChemicalStateChanger& stateChanger = addStateChanger(transformation);
		stateChanger.AddImpactingComponent(Cast<UPrimitiveComponent>(AssociatedComponent.GetComponent(GetOwner())));
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
			currentChangers[transformation].AddImpactingComponent(Cast<UPrimitiveComponent>(comp->AssociatedComponent.GetComponent(comp->GetOwner())));
		else
		{
			ChemicalStateChanger& stateChanger = addStateChanger(transformation);
			stateChanger.AddImpactingComponent(Cast<UPrimitiveComponent>(comp->AssociatedComponent.GetComponent(comp->GetOwner())));
		}
	}
	for (auto& hitComp : hitChemicalComponents)
	{
		EChemicalTransformation transformation = getEffectiveEffect(hitComp.Key->GetType(), hitComp.Key->GetState());
		if (transformation == EChemicalTransformation::None)
			continue;
		if (currentChangers.Contains(transformation))
			currentChangers[transformation].AddImpactingComponent(Cast<UPrimitiveComponent>(hitComp.Key->GetAssociatedComponent()));
		else
		{
			ChemicalStateChanger& stateChanger = addStateChanger(transformation);
			stateChanger.AddImpactingComponent(Cast<UPrimitiveComponent>(hitComp.Key->GetAssociatedComponent()));
		}
	}
}

void	UChemicalComponent::GiveMemory()
{
	//IF ERASED
	//GIVEIDENTITYBACK
}

void	UChemicalComponent::EraseMemory()
{
	//IF IN ZONE< UPDATE VALUES IN ERASE ZONE
	//ELSE NOTHING
}

ChemicalStateChanger&	UChemicalComponent::addStateChanger(EChemicalTransformation transformation)
{
	ChemicalStateChanger	temp(2.0f);
	currentChangers.Add(transformation, temp);
	return currentChangers[transformation];
}
	
UChemicalComponent*	UChemicalComponent::FindAssociatedChemicalComponent(UPrimitiveComponent* referenceComponent)
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
		stateChanger.AddImpactingComponent(Cast<UPrimitiveComponent>(AssociatedComponent.GetComponent(GetOwner())));
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
			currentChangers[transformation].AddImpactingComponent(Cast<UPrimitiveComponent>(comp->AssociatedComponent.GetComponent(comp->GetOwner())));
		else
		{
			ChemicalStateChanger& stateChanger = addStateChanger(transformation);
			stateChanger.AddImpactingComponent(Cast<UPrimitiveComponent>(comp->AssociatedComponent.GetComponent(comp->GetOwner())));
		}
	}
	for (auto& hitComp : hitChemicalComponents)
	{
		EChemicalTransformation transformation = getEffectiveEffect(hitComp.Key->GetType(), hitComp.Key->GetState());
		if (transformation == EChemicalTransformation::None)
			continue;
		if (currentChangers.Contains(transformation))
			currentChangers[transformation].AddImpactingComponent(Cast<UPrimitiveComponent>(hitComp.Key->GetAssociatedComponent()));
		else
		{
			ChemicalStateChanger& stateChanger = addStateChanger(transformation);
			stateChanger.AddImpactingComponent(Cast<UPrimitiveComponent>(hitComp.Key->GetAssociatedComponent()));
		}
	}
}

void	UChemicalComponent::addComponentToChangers(EChemicalTransformation transformation, UPrimitiveComponent* primComponent)
{
	if (!currentChangers.Contains(transformation))
	{
		ChemicalStateChanger& stateChanger = addStateChanger(transformation);
		stateChanger.AddImpactingComponent(primComponent);
	}
	else
		currentChangers[transformation].AddImpactingComponent(primComponent);
}

void	UChemicalComponent::removeComponentFromChangers(EChemicalTransformation transformation, UPrimitiveComponent* primComponent)
{
	if (currentChangers.Contains(transformation))
	{
		ChemicalStateChanger&	transformationStateChanger = currentChangers[transformation];
		transformationStateChanger.RemoveImpactingComponent(primComponent);
		if (transformationStateChanger.GetImpactingComponentsNumber() == 0 && !bPersistantTransformation)
			currentChangers.Remove(transformation);
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