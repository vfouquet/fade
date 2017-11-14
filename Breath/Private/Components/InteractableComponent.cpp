// Fill out your copyright notice in the Description page of Project Settings.

#include "InteractableComponent.h"

#include "GameFramework/Actor.h"

// Sets default values for this component's properties
UInteractableComponent::UInteractableComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UInteractableComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UInteractableComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

UPhysicsConstraintComponent*	UInteractableComponent::AddStickConstraint(UInteractableComponent* hook, UPrimitiveComponent* stickedObject, FName stickedBoneName)
{
	AActor* owner = GetOwner();
	if (!owner)
		return nullptr;
	UPrimitiveComponent* objectPrim = owner->FindComponentByClass<UPrimitiveComponent>();
	if (!objectPrim && !stickedObject)
		return nullptr;
	UPhysicsConstraintComponent* stickConstraint = NewObject<UPhysicsConstraintComponent>(this, TEXT("CustomPhysicConstraint"));
	if (!stickConstraint)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not create physical constraint"));
		return nullptr;
	}
	stickConstraint->SetupAttachment(owner->GetRootComponent());
	stickConstraint->RegisterComponent();
	stickConstraint->SetConstrainedComponents(objectPrim, FName("None"), stickedObject, stickedBoneName);
	//stickConstraint->SetDisableCollision(true);
	
	FStickConstraint	constraint;
	constraint.physicConstraint = stickConstraint;
	constraint.carrier = this;
	constraint.hook = hook;
	stickingConstraints.Add(constraint);
	hook->stickingConstraints.Add(constraint);

	return stickConstraint;
}

void	UInteractableComponent::Unstick()
{
	if (!isSticked)
		return;
	for (int pos = 0; pos < stickingConstraints.Num(); pos++)
	{
		if (this == stickingConstraints[pos].hook)
		{
			stickingConstraints[pos].physicConstraint->BreakConstraint();
			stickingConstraints[pos].physicConstraint->DestroyComponent();
			stickingConstraints[pos].carrier->RemoveHookingConstraint(this);
			stickingConstraints.RemoveAt(pos);
			isSticked = false;
			return;
		}
	}
}

void	UInteractableComponent::RemoveHookingConstraint(UInteractableComponent* hookToRemove)
{
	for (int pos = 0; pos < stickingConstraints.Num(); pos++)
	{
		if (stickingConstraints[pos].hook == hookToRemove)
		{
			stickingConstraints.RemoveAt(pos);
			return;
		}
	}
}