// Fill out your copyright notice in the Description page of Project Settings.

#include "HoldComponent.h"

#include "Engine/World.h"
#include "InteractableComponent.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"
#include "MainCharacter.h"

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
	
	mainCharacter = Cast<AMainCharacter>(GetOwner());
	characterCapsule = GetOwner()->FindComponentByClass<UCapsuleComponent>();
	handleTargetLocation = Cast<UPrimitiveComponent>(HandleTargetLocationReference.GetComponent(GetOwner()));
	leftHandConstraint = Cast<UPhysicsConstraintComponent>(LeftHandPhysicalConstraintReference.GetComponent(GetOwner()));
	rightHandConstraint = Cast<UPhysicsConstraintComponent>(RightHandPhysicalConstraintReference.GetComponent(GetOwner()));
}


// Called every frame
void UHoldComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (notifyingThrow)
	{
		throwNotifyTime += DeltaTime;
		if (throwNotifyTime > ThrowBlockingTimeDebug)
		{
			throwNotifyTime = 0.0f;
			notifyingThrow = false;
			EndThrow();
		}
	}

	closestInteractableObject = nullptr;
	if (!handleTargetLocation && !characterCapsule && !GetOwner())
		return;

	detectInteractableAround();

	if (currentHoldingState == EHoldingState::LightGrabbing)
	{
		if (!holdingObject.IsValid())
		{
			UniversalRelease();
			return;
		}

		handleComponent->SetTargetLocation(handleTargetLocation->GetComponentLocation());
		
		//CHECK HOLDING OBJECT DISTANCE - RELEASE IF TOO FAR
		FVector dist = handleTargetLocation->GetComponentLocation() - holdingPrimitiveComponent->GetComponentLocation();
		
		if (releaseCurrentTime < ReleaseTimeTolerence)
			releaseCurrentTime += DeltaTime;
		else
		{
			if (dist.Size() >= ReleaseDistanceThreshold)
				StopGrab();
		}
		//
	}
	else if (currentHoldingState == EHoldingState::Throwing)
		handleComponent->SetTargetLocation(handleTargetLocation->GetComponentLocation());
	else
		releaseCurrentTime = 0.0f;
}

void	UHoldComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	Super::OnComponentDestroyed(bDestroyingHierarchy);

	if (mainCharacter)
	{
		mainCharacter->UnblockCharacter();
		mainCharacter->DisableMovingHeavyObjectMode();
		mainCharacter->SetThrowingObject(false);
		mainCharacter->SetHoldingObject(false);
	}
}

void	UHoldComponent::Action()
{
	if (currentHoldingState == EHoldingState::LightGrabbing)
	{
		if (closestInteractableObject.IsValid() && closestInteractableObject->CanAcceptStick)
			Stick();
		//else
		//	Throw();
	}
	//else if (currentHoldingState == EHoldingState::HeavyGrabbing)
	//	Throw();
	//THOSE COMMENTS DEPENDS ON THE INPUT BUTTON FOR THROW
}

void	UHoldComponent::Grab()
{
	if (currentHoldingState != EHoldingState::None || mainCharacter->IsInAir())
		return;
	if (!closestInteractableObject.IsValid() || !closestInteractableObject->IsGrabable())
		return;

	if (!closestInteractableObject->IsHeavy)
	{
		currentHoldingState = EHoldingState::LightGrabbing;
		holdingStateChangedDelegate.Broadcast(EHoldingState::None, EHoldingState::LightGrabbing);

		holdingObject = closestInteractableObject.Get();
		holdingObject->SetHoldComponent(this);
		if (holdingObject->IsSticked())
			holdingObject->Unstick();

		holdingPrimitiveComponent = holdingObject->GetAssociatedComponent();
		holdingPrimitiveComponent->SetWorldRotation(characterCapsule->GetComponentRotation().Quaternion());	//RESET ROTATION
		handleComponent->GrabComponentAtLocationWithRotation
			(holdingPrimitiveComponent, "", holdingPrimitiveComponent->GetComponentLocation(), FRotator::ZeroRotator);
		holdingPrimitiveComponent->SetCollisionProfileName("OverlapAllDynamic");	//DISABLE OBJECT COLLISION

		mainCharacter->SetHoldingObject(true);
		mainCharacter->BlockCharacter();
		mainCharacter->PlayLightGrabMontage();
	}
	else
	{
		currentHoldingState = EHoldingState::HeavyGrabbing;

		ACharacter* character = Cast<ACharacter>(characterCapsule->GetOwner());	
		holdingObject = closestInteractableObject.Get();
		holdingObject->SetHoldComponent(this);
		AActor* holdingActor = holdingObject->GetOwner();
		holdingPrimitiveComponent = closestInteractableObject->GetAssociatedComponent();

		FHitResult	hitRes;
		FCollisionQueryParams	queryParams;
		queryParams.AddIgnoredActor(character);
		GetWorld()->LineTraceSingleByChannel(hitRes, character->GetActorLocation(), holdingPrimitiveComponent->GetComponentLocation(), ECollisionChannel::ECC_PhysicsBody, queryParams);
		FRotator newRot = (hitRes.Normal * -1.0f).Rotation();
		newRot.Pitch = 0.0f;
		newRot.Roll = 0.0f;
		FVector	holdPrimExtent = holdingPrimitiveComponent->Bounds.BoxExtent;
		holdPrimExtent.Z = 0.0f;
		FVector	holdingPoint = holdingPrimitiveComponent->GetComponentLocation() + (holdPrimExtent + HoldSnapOffset) * hitRes.Normal;
		
		holdingPrimitiveComponent->SetWorldLocation(holdingPrimitiveComponent->GetComponentLocation() + FVector::UpVector * 10.0f); //ADD SOME Z POSITION TO AVOID GROUND CONTACT (TEMP)

		character->SetActorLocationAndRotation(holdingPoint, newRot, true);	//SNAP CHARACTER WITH A FACING ROTATION

		mainCharacter->EnableMovingHeavyObjectMode();	
		createHandConstraint();
		holdingStateChangedDelegate.Broadcast(EHoldingState::None, EHoldingState::HeavyGrabbing);
	}
}

void	UHoldComponent::EndLightGrab()
{
	mainCharacter->UnblockCharacter();
}

void	UHoldComponent::StopGrab()
{
	if (currentHoldingState == EHoldingState::LightGrabbing)
	{
		releaseLightGrabbedObject();
		mainCharacter->SetHoldingObject(false);
		currentHoldingState = EHoldingState::None;
		holdingStateChangedDelegate.Broadcast(EHoldingState::LightGrabbing, EHoldingState::None);
	}
	else if (currentHoldingState == EHoldingState::HeavyGrabbing)
	{
		holdingStateChangedDelegate.Broadcast(EHoldingState::HeavyGrabbing, EHoldingState::None);
		mainCharacter->DisableMovingHeavyObjectMode();

		releaseHeavyGrabbedObject();

		currentHoldingState = EHoldingState::None;
	}
}

void	UHoldComponent::Throw()
{
	mainCharacter->SetThrowingObject(true);
	if (currentHoldingState == EHoldingState::LightGrabbing)
	{
		currentHoldingState = EHoldingState::Throwing;
		holdingStateChangedDelegate.Broadcast(EHoldingState::LightGrabbing, EHoldingState::Throwing);
	}
	else if (currentHoldingState == EHoldingState::HeavyGrabbing)
	{
		currentHoldingState = EHoldingState::HeavyThrowing;
		holdingStateChangedDelegate.Broadcast(EHoldingState::HeavyGrabbing, EHoldingState::Throwing);
	}
	notifyingThrow = true;
}

void	UHoldComponent::EndThrow()
{
	if (currentHoldingState == EHoldingState::Throwing)
	{
		holdingObject->SetThrown();
		UPrimitiveComponent* tempPrimitive = holdingPrimitiveComponent;
		releaseLightGrabbedObject();
		FRotator	tempRotation = characterCapsule->GetComponentRotation();
		tempRotation.Pitch += AdditionalThrowAngle;
		tempPrimitive->AddImpulse(tempRotation.Vector() * ThrowPower * 10000.0f);
		
		mainCharacter->SetHoldingObject(false);
		holdingStateChangedDelegate.Broadcast(EHoldingState::Throwing, EHoldingState::None);
		currentHoldingState = EHoldingState::None;
	}
	else if (currentHoldingState == EHoldingState::HeavyThrowing)
	{
		UPrimitiveComponent* tempPrimitive = holdingPrimitiveComponent;

		releaseHeavyGrabbedObject();

		tempPrimitive->AddImpulse(characterCapsule->GetForwardVector() * ThrowPower * 10000.0f);
		mainCharacter->DisableMovingHeavyObjectMode();

		holdingStateChangedDelegate.Broadcast(EHoldingState::HeavyThrowing, EHoldingState::None);
		currentHoldingState = EHoldingState::None;
	}
	mainCharacter->SetThrowingObject(false);
}

void	UHoldComponent::Stick()
{
	if (!holdingObject.IsValid() || !closestInteractableObject.IsValid())
		return;
	//ATTACH CONSTRAINT TO OTHER OBJECT
	if (currentHoldingState == EHoldingState::LightGrabbing)
	{
		if (!closestInteractableObject->CanAcceptStick || !holdingObject->CanBeSticked)
			return;

		currentHoldingState = EHoldingState::Sticking;
		mainCharacter->BlockCharacter();

		closestInteractableObject->AddStickConstraint(holdingObject.Get(), holdingPrimitiveComponent, TEXT("None"));
		releaseLightGrabbedObject();
		holdingStateChangedDelegate.Broadcast(EHoldingState::LightGrabbing, EHoldingState::Sticking);

		mainCharacter->UnblockCharacter();
		currentHoldingState = EHoldingState::None;
		holdingStateChangedDelegate.Broadcast(EHoldingState::Sticking, EHoldingState::None);
	}
}

void	UHoldComponent::UniversalRelease()
{
	if (currentHoldingState == EHoldingState::Throwing)
	{
		releaseLightGrabbedObject();
		mainCharacter->SetHoldingObject(false);
		mainCharacter->SetThrowingObject(false);
	}
	else if (currentHoldingState == EHoldingState::HeavyThrowing)
	{
		releaseHeavyGrabbedObject();
		mainCharacter->DisableMovingHeavyObjectMode();
		mainCharacter->SetThrowingObject(false);
	}
	else if (currentHoldingState == EHoldingState::LightGrabbing)
	{
		releaseLightGrabbedObject();
		mainCharacter->SetHoldingObject(false);
	}
	else if (currentHoldingState == EHoldingState::HeavyGrabbing)
	{
		releaseHeavyGrabbedObject();
		mainCharacter->DisableMovingHeavyObjectMode();
	}
	currentHoldingState = EHoldingState::None;
	holdingStateChangedDelegate.Broadcast(currentHoldingState, EHoldingState::None);
}

void	UHoldComponent::releaseLightGrabbedObject()
{
	handleComponent->ReleaseComponent();
	holdingPrimitiveComponent->SetCollisionProfileName("SmallInteractable");
	if (holdingObject.IsValid())
		holdingObject->SetHoldComponent(nullptr);
	holdingObject = nullptr;
	holdingPrimitiveComponent = nullptr;
}

void	UHoldComponent::createHandConstraint()
{
	FVector	center;
	FVector leftSphereLocation;
	FVector rightSphereLocation;
	getPushingPoints(center, leftSphereLocation, rightSphereLocation);	//GET COLSEST POINT ON ACTOR CLOSE TO CHARACTER HANDS

	FVector leftHand = leftHandConstraint->GetComponentLocation();
	FVector rightHand = rightHandConstraint->GetComponentLocation();
	leftHand.Z = leftSphereLocation.Z;
	rightHand.Z = rightSphereLocation.Z;
	leftHandConstraint->SetWorldLocation(leftHand);	//SET CONSTRAINTS TO SAME Z POSITION AS CHARACTER HANDS
	rightHandConstraint->SetWorldLocation(rightHand);

	//leftHandConstraint->SetConstrainedComponents(characterCapsule, "None", holdingObject->CreateLeftConstraintPoint(leftSphereLocation), "None");
	//rightHandConstraint->SetConstrainedComponents(characterCapsule, "None", holdingObject->CreateRightConstraintPoint(rightSphereLocation), "None");
	leftHandConstraint->SetConstrainedComponents(characterCapsule, "None", holdingPrimitiveComponent, "None");
	rightHandConstraint->SetConstrainedComponents(characterCapsule, "None", holdingPrimitiveComponent, "None");
}

void	UHoldComponent::releaseHeavyGrabbedObject()
{
	if (leftHandConstraint)
		leftHandConstraint->BreakConstraint();
	if (rightHandConstraint)
		rightHandConstraint->BreakConstraint();
	if (holdingObject.IsValid())
		holdingObject->SetHoldComponent(nullptr);
	holdingObject = nullptr;
	holdingPrimitiveComponent = nullptr;
}

void	UHoldComponent::detectInteractableAround()
{
	float closestInteractable = FLT_MAX;

	TArray<FHitResult>			hitResults;
	FCollisionQueryParams		queryParams;
	queryParams.AddIgnoredActor(GetOwner());

	GetWorld()->SweepMultiByChannel(hitResults, characterCapsule->GetComponentLocation(),
		characterCapsule->GetComponentLocation() + GetOwner()->GetActorForwardVector() * ((currentHoldingState == EHoldingState::LightGrabbing)? HoldingDetectionOffset : DetectionOffset),
		FQuat::Identity, ECollisionChannel::ECC_GameTraceChannel2,
		FCollisionShape::MakeCapsule(characterCapsule->GetScaledCapsuleRadius(), characterCapsule->GetScaledCapsuleHalfHeight()),
		queryParams);
	
	if (hitResults.Num() > 0)
	{
		for (auto& hitRes : hitResults)
		{
			if (!hitRes.Component.IsValid())
				continue;
			UInteractableComponent* interComp = UInteractableComponent::FindAssociatedInteractableComponent(hitRes.Component.Get());
			if (interComp == holdingObject)
				continue;
			if (interComp)
			{
				float distance = (hitRes.Component->GetComponentLocation() - characterCapsule->GetComponentLocation()).Size();
				if (distance < closestInteractable)
				{
					closestInteractable = distance;
					closestInteractableObject = interComp;
				}
			}
		}
	}
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
	
void	UHoldComponent::DebugInteractableDetection(FColor beginColor, FColor endColor, float lifetime)
{
	DrawDebugCapsule(GetWorld(), characterCapsule->GetComponentLocation(), 
		characterCapsule->GetScaledCapsuleHalfHeight(), 
		characterCapsule->GetScaledCapsuleRadius(), FQuat::Identity, 
		beginColor, false, lifetime);

	FVector destination = characterCapsule->GetComponentLocation() + 
		GetOwner()->GetActorForwardVector() * 
		((currentHoldingState == EHoldingState::LightGrabbing) ? HoldingDetectionOffset : DetectionOffset);
	DrawDebugCapsule(GetWorld(), destination,
		characterCapsule->GetScaledCapsuleHalfHeight(),
		characterCapsule->GetScaledCapsuleRadius(), FQuat::Identity,
		endColor, false, lifetime);
}