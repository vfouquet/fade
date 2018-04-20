// Fill out your copyright notice in the Description page of Project Settings.

#include "ChemicalCeramicComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "Components/DestructibleComponent.h"

void UChemicalCeramicComponent::InitializeComponent()
{
	Super::InitializeComponent();
	type = EChemicalType::Ceramic;

	FScriptDelegate	del;
	del.BindUFunction(this, "OnStateChanged");
	this->stateChangedDelegate.Add(del);
}

ChemicalStateChanger&	UChemicalCeramicComponent::addStateChanger(EChemicalTransformation transformation)
{
	float time = 0.0f;
	if (transformation == EChemicalTransformation::Burning)
	{
		if (state == EChemicalState::None)
			time = BreakByFireTime;
	}
	ChemicalStateChanger	temp(time);
	currentChangers.Add(transformation, temp);
	return currentChangers[transformation];
}

EChemicalTransformation		UChemicalCeramicComponent::getEffectiveEffect(EChemicalType const& otherType, EChemicalState const& otherState) const
{
	if (otherType == EChemicalType::Fire && otherState == EChemicalState::None)
		return EChemicalTransformation::Burning;
	else if ((otherType == EChemicalType::Rock || otherType == EChemicalType::Wood) && otherState == EChemicalState::Burning)
		return EChemicalTransformation::Burning;
	return EChemicalTransformation::None;
}

EChemicalState	UChemicalCeramicComponent::getNextState(EChemicalTransformation const& transformation) const
{
	if (transformation == EChemicalTransformation::Burning)
	{
		if (state == EChemicalState::None)
			return EChemicalState::Extinguished;
	}
	return EChemicalState::None;
}

bool	UChemicalCeramicComponent::computePercussionBreakability(UPrimitiveComponent* other)
{
	/*
	UChemicalComponent*	otherComp = FindAssociatedChemicalComponent(other);
	if (!otherComp || !associatedComponent.Is)
	{

		return false;
	}
	if (associatedComponent->GetComponentVelocity().Size() > SpeedThresholdValue)
		return true;
	if (other->GetComponentVelocity().Size() < SpeedThresholdValue)
		return false;
	if (otherComp->GetType() == EChemicalType::Rock || otherComp->GetType() == EChemicalType::Wood)
		return true;
	*/
	return false;
}
	
void	UChemicalCeramicComponent::OnStateChanged(EChemicalTransformation tranform, EChemicalState previous, EChemicalState next)
{
	if (tranform == EChemicalTransformation::Burning)
	{
		swapTrick();
	}
}

void	UChemicalCeramicComponent::swapTrick()
{
	if (!associatedComponent.IsValid())
	{
		AActor* owner = GetOwner();
		UE_LOG(LogTemp, Warning, TEXT("%s - UChemicalCeramicComponent : Couldn't do swap trick because the associtaed component is nullptr"), owner? *owner->GetName() : *FString("Error"));
		return;
	}
	if (!ActorToSwap)
	{
		AActor* owner = GetOwner();
		UE_LOG(LogTemp, Warning, TEXT("%s - UChemicalCeramicComponent : Couldn't do swap trick because the actor to swap is nullptr"), owner ? *owner->GetName() : *FString("Error"));
		return;
	}

	FVector	linearVel = associatedComponent->GetPhysicsLinearVelocity();
	FVector angularVelocity = associatedComponent->GetPhysicsAngularVelocity();
	
	associatedComponent->SetVisibility(false);

	FTransform	tempTransform;
	tempTransform.SetLocation(associatedComponent->GetComponentLocation());
	tempTransform.SetRotation(associatedComponent->GetComponentQuat());

	FActorSpawnParameters	parameters;
	parameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AActor* newActor = GetWorld()->SpawnActor(ActorToSwap, &tempTransform);
	if (!newActor)
	{
		AActor* owner = GetOwner();
		UE_LOG(LogTemp, Warning, TEXT("%s - UChemicalCeramicComponent : Couldn't do swap trick because the spawned actor is nullptr"), owner ? *owner->GetName() : *FString("Error"));
		return;
	}

	UDestructibleComponent*	destructible = newActor->FindComponentByClass<UDestructibleComponent>();
	if (!destructible)
	{
		AActor* owner = GetOwner();
		UE_LOG(LogTemp, Warning, TEXT("%s - UChemicalCeramicComponent : Couldn't do swap trick because the spawned actor doesn't have a destructible mesh"), owner ? *owner->GetName() : *FString("Error"));
		return;
	}
	destructible->SetPhysicsLinearVelocity(linearVel);
	destructible->SetPhysicsAngularVelocity(angularVelocity);
	destructible->OnComponentFracture = OnDestructibleFracture;

	destructible->ApplyDamage(1.0f, destructible->GetComponentLocation(), FVector::UpVector * -1.0f, 1.0f);

	associatedComponent->DestroyComponent();
}