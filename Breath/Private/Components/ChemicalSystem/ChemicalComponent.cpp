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
	bWantsInitializeComponent = true;
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

	associatedComponent = Cast<UPrimitiveComponent>(AssociatedComponent.GetComponent(owner));
	if (!associatedComponent.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s - %s : Cannot find primitive component, reference is wrong"), *owner->GetName(), *GetName());
		return;
	}

	bindDelegates();
}


// Called every frame
void UChemicalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!associatedComponent.IsValid())
		return;

	if (!bAlreadyTick)
	{
		UE_LOG(LogTemp, Warning, TEXT("NOT TICK"));
		addPropagationComponents();
		refreshChangersWithCurrentInteractions();
		bAlreadyTick = true;
	}

	// ...
	hitChemicalComponents.RemoveAll([&](FChemicalHitData hit) {
		if (hit.chemical.IsValid())
			return hit.chemical->GetAssociatedComponent() == nullptr;
		return true;
	});

	for (auto& hitComp : hitChemicalComponents)
	{
		float distance = FVector::Distance(associatedComponent->GetComponentLocation(), hitComp.chemical->GetAssociatedComponent()->GetComponentLocation());
		if (distance > hitComp.distance + 25.0f)
		{
			for (auto& stateChanger : currentChangers)
			{
				if (stateChanger.Value.RemoveIfNeeded(hitComp.chemical->GetAssociatedComponent()) && !bPersistantTransformation)
					currentChangers.Remove(stateChanger.Key);
			}
		}
	}

	for (auto& propagationComp : propagationComponents)
	{
		if (!propagationComp.bUseDistance)
			continue;
		float distance = FVector::Distance(associatedComponent->GetComponentLocation(), 
			propagationComp.primitive->GetComponentLocation());
		if (distance > propagationComp.initialDistance)
		{
			for (auto& stateChanger : currentChangers)
			{
				if (stateChanger.Value.RemoveIfNeeded(propagationComp.primitive.Get()) && !bPersistantTransformation)
					currentChangers.Remove(stateChanger.Key);
			}
			propagationComponents.Remove(propagationComp);
		}
	}

	for (auto& stateChanger : currentChangers)
	{
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
	
void	UChemicalComponent::OnComponentDestroyed(bool destroyedHierarchy)
{
	/*
	for (auto& hitComp : hitChemicalComponents)
	{
		if (!IsValid() || !hitComp.chemical.IsValid())
			continue;
		float distance = FVector::Distance(associatedComponent->GetComponentLocation(), hitComp.chemical->GetAssociatedComponent()->GetComponentLocation());
		if (distance > hitComp.distance + 25.0f)
		{
			hitChemicalComponents.Remove(hitComp);
			for (auto& stateChanger : currentChangers)
			{
				if (stateChanger.Value.RemoveIfNeeded(hitComp.chemical->GetAssociatedComponent()) && !bPersistantTransformation)
					currentChangers.Remove(stateChanger.Key);
			}
		}
	}
	*/

	for (auto& propagationComp : propagationComponents)
	{
		if (propagationComp.component.IsValid())
			propagationComp.component->removeComponentAllChangers(associatedComponent.Get());
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
	/*
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
		bool contains = hitChemicalComponents.ContainsByPredicate([&](FChemicalHitData hit) {
			if (hit.chemical.IsValid())
				return hit.chemical.Get() == chemicalComp;
			return false;
		});
		if (!contains)
		{
			FChemicalHitData tempHit;
			tempHit.chemical = chemicalComp;
			tempHit.distance = FVector::Distance(HitComponent->GetComponentLocation(), OtherComp->GetComponentLocation());
			hitChemicalComponents.Add(tempHit);
		}
		EChemicalTransformation transformation = getEffectiveEffect(chemicalComp->GetType(), chemicalComp->GetState());
		if (transformation != EChemicalTransformation::None)
			addComponentToChangers(transformation, OtherComp);
	}
	*/
}

void	UChemicalComponent::EraseIdentity()
{
	currentChangers.Empty();
	stateChangedDelegate.Broadcast(EChemicalTransformation::Erasing, state, EChemicalState::NoIdentity);
	state = EChemicalState::NoIdentity;
}

void	UChemicalComponent::GiveIdentity(EChemicalState previousState)
{
	if (state != EChemicalState::NoIdentity)
	{
		AActor* owner = GetOwner();
		UE_LOG(LogTemp, Warning, TEXT("%s - %s : Trying to give identity, but the component already have an identity"), owner ? *owner->GetName() : *FString("Error"), *GetName());
		return;
	}
	state = previousState;
	stateChangedDelegate.Broadcast(EChemicalTransformation::GivingIdentity, EChemicalState::NoIdentity, previousState);
	refreshChangersWithCurrentInteractions();
}

void	UChemicalComponent::AddPropagationComponent(UChemicalComponent* otherChemicalComp)
{
	if (!otherChemicalComp)
	{
		AActor* owner = GetOwner();
		UE_LOG(LogTemp, Warning, TEXT("%s - %s : Cannot add static propagation component, the chemical component is nullptr"), owner ? *owner->GetName() : *FString("Error"), *GetName());
		return;
	}
	FChemicalPropagation	propagationParams;
	propagationParams.component = otherChemicalComp;
	propagationParams.primitive = otherChemicalComp->GetAssociatedComponent();
	propagationComponents.Add(propagationParams);

	propagationParams.component = this;
	propagationParams.primitive = GetAssociatedComponent();
	otherChemicalComp->propagationComponents.Add(propagationParams);
}

void	UChemicalComponent::AddHitComponent(UChemicalComponent* chemicalComp)
{
	/*
	if (!chemicalComp || !chemicalComp->GetAssociatedComponent() || !associatedComponent.IsValid())
		return;
	bool contains = hitChemicalComponents.ContainsByPredicate([&](FChemicalHitData hit) {
		if (hit.chemical.IsValid())
			return hit.chemical.Get() == chemicalComp;
		return false;
	});
	if (!contains)
	{
		FChemicalHitData tempHit;
		tempHit.chemical = chemicalComp;
		tempHit.distance = FVector::Distance(associatedComponent->GetComponentLocation(), chemicalComp->GetAssociatedComponent()->GetComponentLocation());
		hitChemicalComponents.Add(tempHit);
	}
	*/
}

void	UChemicalComponent::OverrideAssociatedComponent(UPrimitiveComponent* newValue)
{
	if (!newValue)
	{
		AActor* owner = GetOwner();
		UE_LOG(LogTemp, Warning, TEXT("%s - %s : Can't override the associated component, given value is nullptr"), owner ? *owner->GetName() : *FString("Error"), *GetName());
		return;
	}
	associatedComponent = newValue;

	bindDelegates();
}

float	UChemicalComponent::GetChangerStatus(EChemicalTransformation transformation, bool& hasTransformation)
{
	float	res = 0.0f;
	if (ChemicalStateChanger* changer = currentChangers.Find(transformation))
	{
		res = changer->GetStatus();
		hasTransformation = true;
	}

	return res;
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
		if (chemicalComp->GetAssociatedComponent() == referenceComponent)
			return chemicalComp;
	}
	return nullptr;
}
	
void	UChemicalComponent::removeComponentAllChangers(UPrimitiveComponent* component)
{
	for (auto& changer : currentChangers)
	{
		if (changer.Value.RemoveIfNeeded(component) && !bPersistantTransformation)
			currentChangers.Remove(changer.Key);
	}
}
	
void	UChemicalComponent::notifyChemicalStateChanged(EChemicalTransformation previousTransformation, EChemicalState previous, EChemicalState next)
{
	stateChangedDelegate.Broadcast(previousTransformation, previous, next);
	refreshChangersWithCurrentInteractions();
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
		if (transformationStateChanger.RemoveIfNeeded(primComponent) && !bPersistantTransformation)
			currentChangers.Remove(transformation);
	}
}
	
void	UChemicalComponent::updateImpact(UChemicalComponent* otherChemical, UPrimitiveComponent* otherPrimitive)
{
	EChemicalTransformation	newTransformation = getEffectiveEffect(otherChemical->GetType(), otherChemical->GetState());

	if (newTransformation != EChemicalTransformation::None)
		addComponentToChangers(newTransformation, otherPrimitive);

	for (auto& changer : currentChangers)
	{
		if (changer.Key == newTransformation)
			continue;
		if (changer.Value.RemoveIfNeeded(otherPrimitive) && !bPersistantTransformation)
			currentChangers.Remove(changer.Key);
	}
}

void	UChemicalComponent::refreshChangersWithCurrentInteractions()
{
	if (!associatedComponent.IsValid())
	{
		DestroyComponent();
		return;
	}

	EChemicalTransformation	transformation = getPotentialSelfNextTransformation();
	if (transformation != EChemicalTransformation::None)
	{
		ChemicalStateChanger& stateChanger = addStateChanger(transformation);
		stateChanger.AddImpactingComponent(associatedComponent.Get());
	}

	TArray<UPrimitiveComponent*>	overlappingPrimitives;
	associatedComponent->GetOverlappingComponents(overlappingPrimitives);
	for (auto It = overlappingPrimitives.CreateConstIterator(); It; ++It)
	{
		UChemicalComponent*	comp = FindAssociatedChemicalComponent(*It);
		if (!comp)
			continue;
		comp->updateImpact(this, associatedComponent.Get());
		EChemicalTransformation transformation = getEffectiveEffect(comp->GetType(), comp->GetState());
		if (transformation == EChemicalTransformation::None)
			continue;
		if (currentChangers.Contains(transformation))
			currentChangers[transformation].AddImpactingComponent(comp->GetAssociatedComponent());
		else
		{
			ChemicalStateChanger& stateChanger = addStateChanger(transformation);
			stateChanger.AddImpactingComponent(comp->GetAssociatedComponent());
		}
	}
	for (auto& hitComp : hitChemicalComponents)
	{
		if (!hitComp.chemical.IsValid())
			continue;
		hitComp.chemical->updateImpact(this, associatedComponent.Get());
		EChemicalTransformation transformation = getEffectiveEffect(hitComp.chemical->GetType(), hitComp.chemical->GetState());
		if (transformation == EChemicalTransformation::None)
			continue;
		if (currentChangers.Contains(transformation))
			currentChangers[transformation].AddImpactingComponent(hitComp.chemical->GetAssociatedComponent());
		else
		{
			ChemicalStateChanger& stateChanger = addStateChanger(transformation);
			stateChanger.AddImpactingComponent(hitComp.chemical->GetAssociatedComponent());
		}
	}

	for (auto& propagationComp : propagationComponents)
	{
		propagationComp.component->updateImpact(this, associatedComponent.Get());
		EChemicalTransformation transformation = getEffectiveEffect(propagationComp.component->GetType(), propagationComp.component->GetState());
		if (transformation == EChemicalTransformation::None)
			continue;
		if (currentChangers.Contains(transformation))
			currentChangers[transformation].AddImpactingComponent(propagationComp.component->GetAssociatedComponent());
		else
		{
			ChemicalStateChanger& stateChanger = addStateChanger(transformation);
			stateChanger.AddImpactingComponent(propagationComp.component->GetAssociatedComponent());
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

void	UChemicalComponent::bindDelegates()
{
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

void	UChemicalComponent::addPropagationComponents()
{
	for (auto& propagationComp : PropagationComponentReferences)
	{
		UPrimitiveComponent* otherPrimitive = Cast<UPrimitiveComponent>(propagationComp.GetComponent(propagationComp.OtherActor));
		if (!otherPrimitive)
		{
			AActor* owner = GetOwner();
			UE_LOG(LogTemp, Warning, TEXT("%s - %s : Cannot add propagation component, the primitive reference is wrong"), owner? *owner->GetName() : *FString("Error"), *GetName());
			continue;
		}
		UChemicalComponent* otherChemicalComp = FindAssociatedChemicalComponent(otherPrimitive);
		if (otherChemicalComp)
		{
			AActor* owner = GetOwner();
			UE_LOG(LogTemp, Warning, TEXT("%s - %s : Cannot add propagation component, the primitive reference doesn't have chemical component associated"), owner ? *owner->GetName() : *FString("Error"), *GetName());
			continue;
		}
		FChemicalPropagation	propagationParams;
		propagationParams.component = otherChemicalComp;
		propagationParams.primitive = otherPrimitive;
		propagationParams.bUseDistance = true;
		propagationParams.initialDistance =
			FVector::Distance(associatedComponent->GetComponentLocation(), otherPrimitive->GetComponentLocation());
		propagationComponents.Add(propagationParams);

		propagationParams.component = this;
		propagationParams.primitive = GetAssociatedComponent();
		otherChemicalComp->propagationComponents.Add(propagationParams);
	}
	for (auto& propagationComp : StaticPropagationComponentReferences)
	{
		UPrimitiveComponent* otherPrimitive = Cast<UPrimitiveComponent>(propagationComp.GetComponent(propagationComp.OtherActor));
		if (!otherPrimitive)
		{
			AActor* owner = GetOwner();
			UE_LOG(LogTemp, Warning, TEXT("%s - %s : Cannot add static propagation component, the primitive reference is wrong"), owner ? *owner->GetName() : *FString("Error"), *GetName());
			continue;
		}
		UChemicalComponent* otherChemicalComp = FindAssociatedChemicalComponent(otherPrimitive);
		if (!otherChemicalComp)
		{
			AActor* owner = GetOwner();
			UE_LOG(LogTemp, Warning, TEXT("%s - %s : Cannot add static propagation component, the primitive reference doesn't have chemical component associated"), owner ? *owner->GetName() : *FString("Error"), *GetName());
			continue;
		}
		FChemicalPropagation	propagationParams;
		propagationParams.component = otherChemicalComp;
		propagationParams.primitive = otherPrimitive;
		propagationComponents.Add(propagationParams);

		propagationParams.component = this;
		propagationParams.primitive = GetAssociatedComponent();
		otherChemicalComp->propagationComponents.Add(propagationParams);
	}
	PropagationComponentReferences.Empty();
	StaticPropagationComponentReferences.Empty();
}