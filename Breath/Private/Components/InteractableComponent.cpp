// Fill out your copyright notice in the Description page of Project Settings.

#include "InteractableComponent.h"

#include "GameFramework/Actor.h"

// Sets default values for this component's properties
UInteractableComponent::UInteractableComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UInteractableComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Grab.ComponentProperty != NAME_None)
	{
		UPrimitiveComponent* grabComp = Cast<UPrimitiveComponent>(Grab.GetComponent(GetOwner()));
		FScriptDelegate	hitOverlap;
		hitOverlap.BindUFunction(this, "OnHit");
		grabComp->OnComponentHit.Add(hitOverlap);
	}

	// ...
	leftConstraintPoint = NewObject<USphereComponent>(this, TEXT("LeftConstraintPoint"));
	rightConstraintPoint = NewObject<USphereComponent>(this, TEXT("RightConstraintPoint"));
	leftConstraintPoint->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);
	rightConstraintPoint->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);
	leftConstraintPoint->SetActive(false);
	rightConstraintPoint->SetActive(false);
	leftConstraintPoint->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	rightConstraintPoint->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	leftConstraintPoint->SetSphereRadius(10.0f, true);
	rightConstraintPoint->SetSphereRadius(10.0f, true);
	leftConstraintPoint->SetSimulatePhysics(true);
	rightConstraintPoint->SetSimulatePhysics(true);

	leftConstraint = NewObject<UPhysicsConstraintComponent>(this, TEXT("LeftConstraint"));
	rightConstraint = NewObject<UPhysicsConstraintComponent>(this, TEXT("RightConstraint"));
	leftConstraint->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);
	rightConstraint->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);
	leftConstraint->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Locked, 0.0f);
	leftConstraint->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Locked, 0.0f);
	leftConstraint->SetAngularTwistLimit(EAngularConstraintMotion::ACM_Locked, 0.0f);
	rightConstraint->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Locked, 0.0f);
	rightConstraint->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Locked, 0.0f);
	rightConstraint->SetAngularTwistLimit(EAngularConstraintMotion::ACM_Locked, 0.0f);
	leftConstraint->SetActive(false);
	rightConstraint->SetActive(false);

	
	UPrimitiveComponent*	primitiveComp = GetOwner()->FindComponentByClass<UPrimitiveComponent>();
	float halfMass = primitiveComp->GetMass() * 0.5f;

	FVector dist = primitiveComp->GetComponentLocation() - leftConstraintPoint->GetComponentLocation();
	FVector dist2 = primitiveComp->GetComponentLocation() - rightConstraintPoint->GetComponentLocation();
	
	leftConstraint->SetLinearXLimit(ELinearConstraintMotion::LCM_Limited, dist.Size());
	leftConstraint->SetLinearYLimit(ELinearConstraintMotion::LCM_Limited, dist.Size());
	leftConstraint->SetLinearZLimit(ELinearConstraintMotion::LCM_Limited, dist.Size());
	rightConstraint->SetLinearXLimit(ELinearConstraintMotion::LCM_Limited, dist2.Size());
	rightConstraint->SetLinearYLimit(ELinearConstraintMotion::LCM_Limited, dist2.Size());
	rightConstraint->SetLinearZLimit(ELinearConstraintMotion::LCM_Limited, dist2.Size());
	
	leftConstraintPoint->SetMassOverrideInKg("None", halfMass, true);
	rightConstraintPoint->SetMassOverrideInKg("None", halfMass, true);
}


// Called every frame
void UInteractableComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void	UInteractableComponent::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!thrown)
		return;
	if (thrown)
		thrown = false;
	if (!OtherActor || isSticked)
		return;
	UInteractableComponent*	otherInteractble = OtherActor->FindComponentByClass<UInteractableComponent>();
	if (!otherInteractble)
		return;
	if (otherInteractble->CanAcceptStick)
		otherInteractble->AddStickConstraint(this, HitComponent, NAME_None);
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
	hook->isSticked = true;

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