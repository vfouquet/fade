// Fill out your copyright notice in the Description page of Project Settings.

#include "InteractableComponent.h"

#include "GameFramework/Actor.h"

// Sets default values for this component's properties
UInteractableComponent::UInteractableComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	leftConstraintPoint = CreateDefaultSubobject<USphereComponent>(TEXT("LeftConstraintPoint"));
	rightConstraintPoint = CreateDefaultSubobject<USphereComponent>(TEXT("RightConstraintPoint"));
	
	leftConstraint = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("LeftConstraint"));
	rightConstraint = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("RightConstraint"));
	// ...
}


// Called when the game starts
void UInteractableComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
	UPrimitiveComponent*	primitiveComp = GetOwner()->FindComponentByClass<UPrimitiveComponent>();
	FAttachmentTransformRules	rules(EAttachmentRule::KeepWorld, true);

	float halfMass = primitiveComp->GetMass() * 0.5f;

	leftConstraintPoint->AttachToComponent(primitiveComp, rules);
	leftConstraintPoint->SetSphereRadius(10.0f, true);
	leftConstraintPoint->SetMassOverrideInKg("None", halfMass, true);
	leftConstraintPoint->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	leftConstraintPoint->SetSimulatePhysics(true);
	leftConstraintPoint->SetActive(false);

	rightConstraintPoint->AttachToComponent(primitiveComp, rules);
	rightConstraintPoint->SetSphereRadius(10.0f, true);
	rightConstraintPoint->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	rightConstraintPoint->SetMassOverrideInKg("None", halfMass, true);
	rightConstraintPoint->SetSimulatePhysics(true);
	rightConstraintPoint->SetActive(false);

	leftConstraint->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Locked, 0.0f);
	leftConstraint->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Locked, 0.0f);//22.5f);
	leftConstraint->SetAngularTwistLimit(EAngularConstraintMotion::ACM_Locked, 0.0f);
	leftConstraint->AttachToComponent(primitiveComp, rules);
	leftConstraint->SetActive(false);
	rightConstraint->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Locked, 0.0f);
	rightConstraint->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Locked, 0.0f);//22.5f);
	rightConstraint->SetAngularTwistLimit(EAngularConstraintMotion::ACM_Locked, 0.0f);
	rightConstraint->AttachToComponent(primitiveComp, rules);
	rightConstraint->SetActive(false);
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

UPrimitiveComponent*	UInteractableComponent::CreateLeftConstraintPoint(FVector location)
{
	UPrimitiveComponent*	primitiveComp = GetOwner()->FindComponentByClass<UPrimitiveComponent>();
	leftConstraintPoint->SetWorldLocation(location);
	leftConstraintPoint->SetActive(true);
	leftConstraint->SetWorldLocation(location);
	leftConstraint->SetConstrainedComponents(primitiveComp, "None", leftConstraintPoint, "None");
	leftConstraint->SetActive(true);

	return leftConstraintPoint;
}

UPrimitiveComponent*	UInteractableComponent::CreateRightConstraintPoint(FVector location)
{
	UPrimitiveComponent*	primitiveComp = GetOwner()->FindComponentByClass<UPrimitiveComponent>();
	rightConstraintPoint->SetWorldLocation(location);
	rightConstraintPoint->SetActive(true);
	rightConstraint->SetWorldLocation(location);
	rightConstraint->SetConstrainedComponents(primitiveComp, "None", rightConstraintPoint, "None");
	rightConstraint->SetActive(true);

	return rightConstraintPoint;
}

void	UInteractableComponent::ReleaseLeftRightConstraintPoint()
{
	leftConstraint->BreakConstraint();
	leftConstraintPoint->SetActive(false);
	leftConstraint->SetActive(false);
	rightConstraint->BreakConstraint();
	rightConstraintPoint->SetActive(false);
	rightConstraint->SetActive(false);
	/*
	if (leftConstraintPoint)
	{
		leftConstraintPoint->DestroyComponent();
		leftConstraintPoint = nullptr;
	}
	if (rightConstraintPoint)
	{
		rightConstraintPoint->DestroyComponent();
		rightConstraintPoint = nullptr;
	}
	*/
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