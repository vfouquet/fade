// Fill out your copyright notice in the Description page of Project Settings.

#include "HoldComponent.h"

#include "Engine/World.h"
#include "InteractableComponent.h"
#include "DrawDebugHelpers.h"

#ifdef WITH_EDITOR
#include "UnrealEd.h"
#endif

// Sets default values for this component's properties
UHoldComponent::UHoldComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	handleComponent = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicHandleComponent"));
	// ...
}


// Called when the game starts
void UHoldComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
	characterCapsule = GetOwner()->FindComponentByClass<UCapsuleComponent>();
	characterMoveComponent = GetOwner()->FindComponentByClass<UMoveComponent>();
	handleTargetLocation = Cast<UPrimitiveComponent>(HandleTargetLocationReference.GetComponent(GetOwner()));
	leftHandConstraint = Cast<UPhysicsConstraintComponent>(LeftHandPhysicalConstraintReference.GetComponent(GetOwner()));
	rightHandConstraint = Cast<UPhysicsConstraintComponent>(RightHandPhysicalConstraintReference.GetComponent(GetOwner()));
}


// Called every frame
void UHoldComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	closestInteractableObject = nullptr;
	if (!handleTargetLocation && !characterCapsule)
		return;
	// ...
	float closestInteractable = FLT_MAX;

	TArray<FHitResult>			hitResults;
	FCollisionShape				shape;
	FCollisionQueryParams		queryParams;
	FCollisionResponseParams	responseParam;
	FQuat						quaternion = characterCapsule->GetComponentQuat();
	shape.SetCapsule(characterCapsule->GetScaledCapsuleRadius(), characterCapsule->GetScaledCapsuleHalfHeight());

	if (GetWorld()->SweepMultiByChannel(hitResults, handleTargetLocation->GetComponentLocation(),
		handleTargetLocation->GetComponentLocation() + GetOwner()->GetActorForwardVector() * DetectionOffset, quaternion,
		ECC_WorldDynamic, shape, queryParams, responseParam))
	{	
		for (auto& hitRes : hitResults)
		{
			if (!hitRes.Actor.IsValid())
				continue;
			UInteractableComponent* interComp = hitRes.Actor->FindComponentByClass<UInteractableComponent>();
			if (interComp == holdingObject)
				continue;
			if (interComp)
			{
				float distance = (hitRes.Actor->GetActorLocation() - characterCapsule->GetComponentLocation()).Size();
				if (distance < closestInteractable)
				{
					closestInteractable = distance;
					closestInteractableObject = interComp;
				}
			}
		}
	}

	if (currentHoldingState == EHoldingState::LightGrabbing)
		handleComponent->SetTargetLocation(handleTargetLocation->GetComponentLocation());
	else if (currentHoldingState == EHoldingState::HeavyGrabbing)
	{
		ACharacter*	charac = Cast<ACharacter>(GetOwner());
		if (!charac)
			return;
		UMoveComponent* moveComp = charac->FindComponentByClass<UMoveComponent>();
		if (moveComp)
			moveComp->SetHoldingObjectLocation(holdingPrimitiveComponent->GetComponentLocation());
	}
#ifdef WITH_EDITOR
	ACharacter*	charac = Cast<ACharacter>(GetOwner());
	if (!charac)
		return;
	APlayerController*	cont = Cast<APlayerController>(charac->GetController());
	if (cont->IsInputKeyDown(EKeys::G))
		GUnrealEd->PlayWorld->bDebugPauseExecution = true;
#endif
}

void	UHoldComponent::Action()
{
	if (currentHoldingState == EHoldingState::LightGrabbing)
	{
		if (closestInteractableObject.IsValid() && closestInteractableObject->CanAcceptStick)
			Stick();
		else
			Throw();
	}
	else if (currentHoldingState == EHoldingState::HeavyGrabbing)
		Throw();
}

void	UHoldComponent::Grab()
{
	if (currentHoldingState != EHoldingState::None)
		return;
	if (!closestInteractableObject.IsValid())
		return;

	if (!closestInteractableObject->CanBeGrabbed)
		return;

	if (!closestInteractableObject->IsHeavy)
	{
		currentHoldingState = EHoldingState::LightGrabbing;
		holdingStateChangedDelegate.Broadcast(EHoldingState::None, EHoldingState::LightGrabbing);

		holdingObject = closestInteractableObject.Get();
		if (holdingObject->IsSticked())
			holdingObject->Unstick();
		AActor*	holdingActor = holdingObject->GetOwner();

		//MAYBE QLWAYS TRY THE ROOT AS PRIMITIVE
		holdingPrimitiveComponent =
			holdingActor->FindComponentByClass<UPrimitiveComponent>();
		handleComponent->GrabComponentAtLocationWithRotation
		(holdingPrimitiveComponent, "", holdingActor->GetActorLocation(), holdingActor->GetActorRotation());
	}
	else
	{
		currentHoldingState = EHoldingState::HeavyGrabbing;

		ACharacter* character = Cast<ACharacter>(characterCapsule->GetOwner());	
		holdingObject = closestInteractableObject.Get();
		AActor* holdingActor = holdingObject->GetOwner();
		holdingPrimitiveComponent = holdingActor->FindComponentByClass<UPrimitiveComponent>();

		FHitResult	hitRes;
		FCollisionQueryParams	queryParams;
		queryParams.AddIgnoredActor(character);
		GetWorld()->LineTraceSingleByChannel(hitRes, character->GetActorLocation(), holdingPrimitiveComponent->GetComponentLocation(), ECollisionChannel::ECC_PhysicsBody, queryParams);
		FRotator newRot = (hitRes.Normal * -1.0f).Rotation();
		FVector	holdPrimExtent = holdingPrimitiveComponent->Bounds.BoxExtent;
		holdPrimExtent.Z = 0.0f;
		FVector	holdingPoint = holdingPrimitiveComponent->GetComponentLocation() + (holdPrimExtent + HoldSnapOffset) * hitRes.Normal;

		character->SetActorLocationAndRotation(holdingPoint, newRot, true);

		if (characterMoveComponent)
			characterMoveComponent->EnableMovingHeavyObjectMode();
	
		createHandConstraint();

		holdingStateChangedDelegate.Broadcast(EHoldingState::None, EHoldingState::HeavyGrabbing);
	}
}

void	UHoldComponent::StopGrab()
{
	if (currentHoldingState == EHoldingState::LightGrabbing)
	{
		releaseLightGrabbedObject();
		currentHoldingState = EHoldingState::None;
		holdingStateChangedDelegate.Broadcast(EHoldingState::LightGrabbing, EHoldingState::None);
	}
	else if (currentHoldingState == EHoldingState::HeavyGrabbing)
	{
		holdingStateChangedDelegate.Broadcast(EHoldingState::HeavyGrabbing, EHoldingState::None);
		if (characterMoveComponent)
			characterMoveComponent->DisableMovingHeavyObjectMode();

		releaseHeavyGrabbedObject();

		currentHoldingState = EHoldingState::None;
	}
}

void	UHoldComponent::Throw()
{
	if (currentHoldingState == EHoldingState::LightGrabbing)
	{
		currentHoldingState = EHoldingState::Throwing;
		holdingStateChangedDelegate.Broadcast(EHoldingState::LightGrabbing, EHoldingState::Throwing);

		//DO THIS AT THE END OF ANIMATION (NOTIFY)
		UPrimitiveComponent* tempPrimitive = holdingPrimitiveComponent;
		releaseLightGrabbedObject();
		tempPrimitive->AddImpulse(characterCapsule->GetForwardVector() * ThrowPower);
		holdingStateChangedDelegate.Broadcast(EHoldingState::Throwing, EHoldingState::None);
		currentHoldingState = EHoldingState::None;
	}
	else if (currentHoldingState == EHoldingState::HeavyGrabbing)
	{
		currentHoldingState = EHoldingState::Throwing;
		holdingStateChangedDelegate.Broadcast(EHoldingState::HeavyGrabbing, EHoldingState::Throwing);

		//DO THIS AT THE END OF ANIMATION (NOTIFY)
		UPrimitiveComponent* tempPrimitive = holdingPrimitiveComponent;

		releaseHeavyGrabbedObject();

		tempPrimitive->AddImpulse(characterCapsule->GetForwardVector() * ThrowPower);
		if (characterMoveComponent)
			characterMoveComponent->DisableMovingHeavyObjectMode();

		holdingStateChangedDelegate.Broadcast(EHoldingState::Throwing, EHoldingState::None);
		currentHoldingState = EHoldingState::None;
	}
}

void	UHoldComponent::Stick()
{
	if (!holdingObject || !closestInteractableObject.IsValid())
		return;
	//ATTACH CONSTRAINT TO OTHER OBJECT
	if (currentHoldingState == EHoldingState::LightGrabbing)
	{
		if (!closestInteractableObject->CanAcceptStick || !holdingObject->CanBeSticked)
			return;

		currentHoldingState = EHoldingState::Sticking;
		if (characterMoveComponent)
			characterMoveComponent->BlockCharacter();

		holdingObject->SetStickingActivated();
		closestInteractableObject->AddStickConstraint(holdingObject, holdingPrimitiveComponent, TEXT("None"));
		releaseLightGrabbedObject();
		holdingStateChangedDelegate.Broadcast(EHoldingState::LightGrabbing, EHoldingState::Sticking);

		if (characterMoveComponent)
			characterMoveComponent->UnblockCharacter();
		currentHoldingState = EHoldingState::None;
		holdingStateChangedDelegate.Broadcast(EHoldingState::Sticking, EHoldingState::None);
	}
}

void	UHoldComponent::releaseLightGrabbedObject()
{
	handleComponent->ReleaseComponent();
	holdingObject = nullptr;
	holdingPrimitiveComponent = nullptr;
}

void	UHoldComponent::createHandConstraint()
{
	FVector	center;
	FVector leftSphereLocation;
	FVector rightSphereLocation;
	getPushingPoints(center, leftSphereLocation, rightSphereLocation);

	leftHandConstraint->SetConstrainedComponents(characterCapsule, "None", holdingObject->CreateLeftConstraintPoint(leftSphereLocation), "None");
	rightHandConstraint->SetConstrainedComponents(characterCapsule, "None", holdingObject->CreateRightConstraintPoint(rightSphereLocation), "None");
	//leftHandConstraint->SetConstrainedComponents(characterCapsule, "None", holdingPrimitiveComponent, "None");
	//rightHandConstraint->SetConstrainedComponents(characterCapsule, "None", holdingPrimitiveComponent, "None");
}

void	UHoldComponent::releaseHeavyGrabbedObject()
{
	if (leftHandConstraint)
		leftHandConstraint->BreakConstraint();
	if (rightHandConstraint)
		rightHandConstraint->BreakConstraint();
	holdingObject = nullptr;
	holdingPrimitiveComponent = nullptr;
}

bool	UHoldComponent::getPushingPoints(FVector& centerPoint, FVector& firstPoint, FVector& secondPoint) const
{
	float dis = holdingPrimitiveComponent->GetClosestPointOnCollision(characterCapsule->GetOwner()->GetActorLocation(), centerPoint);
	if (dis == 0.0f)
		return false;
	FVector direction = centerPoint - characterCapsule->GetOwner()->GetActorLocation();
	FVector normal = FVector::CrossProduct(direction, characterCapsule->GetOwner()->GetActorUpVector()).GetUnsafeNormal();
	firstPoint = centerPoint + normal * characterCapsule->GetScaledCapsuleRadius();
	secondPoint = centerPoint - normal * characterCapsule->GetScaledCapsuleRadius();
	DrawDebugSphere(GetWorld(), firstPoint, 10.0f, 12, FColor::Emerald, true, 5.0f);
	DrawDebugSphere(GetWorld(), secondPoint, 10.0f, 12, FColor::Turquoise, true, 5.0f);
	return true;
}