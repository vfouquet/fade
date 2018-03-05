// Fill out your copyright notice in the Description page of Project Settings.

#include "InteractableComponent.h"

#include "GameFramework/Actor.h"
#include "HoldComponent.h"

#include "DrawDebugHelpers.h"
#include "Engine/World.h"

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
		associatedComponent = Cast<UPrimitiveComponent>(Grab.GetComponent(GetOwner()));
		FScriptDelegate	hitOverlap;
		hitOverlap.BindUFunction(this, "OnHit");
		associatedComponent->OnComponentHit.Add(hitOverlap);

		tempExtent = associatedComponent->Bounds.BoxExtent;
	}
}


// Called every frame
void UInteractableComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool	UInteractableComponent::GetDebugLeft()
{
	if (!associatedComponent)
		return false;

	FVector tempExtent2 = tempExtent * 1.0f;

	FCollisionShape shape = FCollisionShape::MakeBox(FVector(tempExtent2.X, 10.0f, tempExtent2.Z));
	FHitResult hitResult;

	FVector leftCenter = associatedComponent->GetComponentLocation() + associatedComponent->GetRightVector() * -tempExtent.Y;
	FCollisionQueryParams	params;
	params.AddIgnoredActor(GetOwner());
	DrawDebugBox(GetWorld(), leftCenter, FVector(tempExtent2.X, 10.0f, tempExtent2.Z), associatedComponent->GetComponentRotation().Quaternion(), FColor::Blue);
	bool res = GetWorld()->SweepSingleByChannel(hitResult, leftCenter, leftCenter, associatedComponent->GetComponentRotation().Quaternion(), ECollisionChannel::ECC_Visibility, shape, params);
	return res;
}

bool	UInteractableComponent::CanRotateLeft(FVector characterForward)
{
	if (!associatedComponent)
		return false;
	bool res = false;
	float dotRes = FVector::DotProduct(characterForward, associatedComponent->GetForwardVector());
	if (FMath::IsNearlyEqual(dotRes, 1.0f, 0.1f))
		res = !GetDebugLeft();
	else if (FMath::IsNearlyEqual(dotRes, -1.0f, 0.1f))
		res = !GetDebugRight();
	else
	{
		float dotResRight = FVector::DotProduct(characterForward, associatedComponent->GetRightVector());
		if (FMath::IsNearlyEqual(dotResRight, 1.0f, 0.1f))
			res = !GetDebugCenter();
		else if (FMath::IsNearlyEqual(dotResRight, -1.0f, 0.1f))
			res = !GetDebugBack();
	}
	return res;
}

bool	UInteractableComponent::CanRotateRight(FVector characterForward)
{
	if (!associatedComponent)
		return false;
	bool res = false;
	float dotRes = FVector::DotProduct(characterForward, associatedComponent->GetForwardVector());
	if (FMath::IsNearlyEqual(dotRes, 1.0f, 0.1f))
		res = !GetDebugRight();
	else if (FMath::IsNearlyEqual(dotRes, -1.0f, 0.1f))
		res = !GetDebugLeft();
	else
	{
		float dotResRight = FVector::DotProduct(characterForward, associatedComponent->GetRightVector());
		if (FMath::IsNearlyEqual(dotResRight, 1.0f, 0.1f))
			res = !GetDebugBack();
		else if (FMath::IsNearlyEqual(dotResRight, -1.0f, 0.1f))
			res = !GetDebugCenter();
	}
	return res;
}

bool	UInteractableComponent::CanPushForward(FVector characterForward)
{
	if (!associatedComponent)
		return false;
	bool res = false;
	float dotRes = FVector::DotProduct(characterForward, associatedComponent->GetForwardVector());
	if (FMath::IsNearlyEqual(dotRes, 1.0f, 0.1f))
		res = !GetDebugCenter();
	else if (FMath::IsNearlyEqual(dotRes, -1.0f, 0.1f))
		res = !GetDebugBack();
	else
	{
		float dotResRight = FVector::DotProduct(characterForward, associatedComponent->GetRightVector());
		if (FMath::IsNearlyEqual(dotResRight, 1.0f, 0.1f))
			res = !GetDebugRight();
		else if (FMath::IsNearlyEqual(dotResRight, -1.0f, 0.1f))
			res = !GetDebugLeft();
	}
	return res;
}

bool	UInteractableComponent::GetDebugRight()
{
	if (!associatedComponent)
		return false;

	FVector	tempExtent2 = tempExtent * 1.0f;

	FCollisionShape shape = FCollisionShape::MakeBox(FVector(tempExtent2.X, 10.0f, tempExtent2.Z));
	FHitResult hitResult;

	FVector rightCenter = associatedComponent->GetComponentLocation() + associatedComponent->GetRightVector() * tempExtent.Y;
	FCollisionQueryParams	params;
	params.AddIgnoredActor(GetOwner());
	DrawDebugBox(GetWorld(), rightCenter, FVector(tempExtent2.X, 10.0f, tempExtent2.Z), associatedComponent->GetComponentRotation().Quaternion(), FColor::Blue);
	bool res = GetWorld()->SweepSingleByChannel(hitResult, rightCenter, rightCenter, associatedComponent->GetComponentRotation().Quaternion(), ECollisionChannel::ECC_Visibility, shape, params);
	return res;
}

bool	UInteractableComponent::GetDebugCenter()
{
	if (!associatedComponent)
		return false;

	FVector	tempExtent2 = tempExtent * 1.0f;

	FCollisionShape shape = FCollisionShape::MakeBox(FVector(10.0f, tempExtent2.Y, tempExtent2.Z));
	FHitResult hitResult;

	FVector forwardCenter = associatedComponent->GetComponentLocation() + associatedComponent->GetForwardVector() * tempExtent.X;
	FCollisionQueryParams	params;
	params.AddIgnoredActor(GetOwner());
	DrawDebugBox(GetWorld(), forwardCenter, FVector(10.0f, tempExtent2.Y, tempExtent2.Z), associatedComponent->GetComponentRotation().Quaternion(), FColor::Blue);
	bool res = GetWorld()->SweepSingleByChannel(hitResult, forwardCenter, forwardCenter, associatedComponent->GetComponentRotation().Quaternion(), ECollisionChannel::ECC_Visibility, shape, params);
	return res;
}

bool	UInteractableComponent::GetDebugBack()
{
	if (!associatedComponent)
		return false;

	FVector	tempExtent2 = tempExtent * 1.0f;

	FCollisionShape shape = FCollisionShape::MakeBox(FVector(10.0f, tempExtent2.Y, tempExtent2.Z));
	FHitResult hitResult;

	FVector forwardCenter = associatedComponent->GetComponentLocation() - associatedComponent->GetForwardVector() * tempExtent.X;
	FCollisionQueryParams	params;
	params.AddIgnoredActor(GetOwner());
	DrawDebugBox(GetWorld(), forwardCenter, FVector(10.0f, tempExtent2.Y, tempExtent2.Z), associatedComponent->GetComponentRotation().Quaternion(), FColor::Blue);
	bool res = GetWorld()->SweepSingleByChannel(hitResult, forwardCenter, forwardCenter, associatedComponent->GetComponentRotation().Quaternion(), ECollisionChannel::ECC_Visibility, shape, params);
	return res;
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

void	UInteractableComponent::OnComponentDestroyed(bool bDestroyHierarchy)
{
	Super::OnComponentDestroyed(bDestroyHierarchy);

	if (holder)
	{
		holder->UniversalRelease();
		holder = nullptr;
	}
	for (auto& constraint : stickingConstraints)
	{
		constraint.physicConstraint->BreakConstraint();
		constraint.physicConstraint->DestroyComponent();
		constraint.carrier->RemoveHookingConstraint(this);
	}
	stickingConstraints.Empty();
}

void	UInteractableComponent::EraseIdentity()
{
	identityErased = true;
	if (holder)
	{
		holder->UniversalRelease();
		holder = nullptr;
	}
}

UPhysicsConstraintComponent*	UInteractableComponent::AddStickConstraint(UInteractableComponent* hook, UPrimitiveComponent* stickedObject, FName stickedBoneName)
{
	if (!associatedComponent && !stickedObject)
		return nullptr;
	UPhysicsConstraintComponent* stickConstraint = NewObject<UPhysicsConstraintComponent>(this, TEXT("CustomPhysicConstraint"));
	if (!stickConstraint)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not create physical constraint"));
		return nullptr;
	}
	stickConstraint->SetupAttachment(this);
	stickConstraint->RegisterComponent();
	stickConstraint->SetConstrainedComponents(associatedComponent, FName("None"), stickedObject, stickedBoneName);
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

UInteractableComponent* UInteractableComponent::FindAssociatedInteractableComponent(UPrimitiveComponent* referenceComponent)
{
	AActor* refCompOwner = referenceComponent->GetOwner();
	if (!refCompOwner)	return nullptr;

	TArray<UActorComponent*>	interactableComponents = refCompOwner->GetComponentsByClass(UInteractableComponent::StaticClass());
	for (auto& actorComp : interactableComponents)
	{
		UInteractableComponent*	interactableComp = Cast<UInteractableComponent>(actorComp);
		if (!interactableComp) continue;
		UPrimitiveComponent* tempPrimitive = Cast<UPrimitiveComponent>(interactableComp->Grab.GetComponent(refCompOwner));
		if (tempPrimitive == referenceComponent)
			return interactableComp;
	}
	return nullptr;
}