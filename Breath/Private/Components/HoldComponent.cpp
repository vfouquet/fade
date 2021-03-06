// Fill out your copyright notice in the Description page of Project Settings.

#include "HoldComponent.h"
#include "AkAudio/Classes/AkGameplayStatics.h"

#include "Engine/World.h"
#include "InteractableComponent.h"
#include "GameFramework/Character.h"
#include "MainCharacter.h"

// Sets default values for this component's properties
UHoldComponent::UHoldComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	//handleComponent = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicHandleComponent"));
	// ...
}


// Called when the game starts
void UHoldComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
	mainCharacter = Cast<AMainCharacter>(GetOwner());
	characterCapsule = GetOwner()->FindComponentByClass<UCapsuleComponent>();
}


// Called every frame
void UHoldComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	closestInteractableObject = nullptr;
	if (!characterCapsule && !GetOwner())
		return;

	pendingReleasedPrimitives.RemoveAll([&](FPendingPrimitive value) {
		if (!value.primitive.IsValid())
			return true;
		TArray<UPrimitiveComponent*>	overlappings;
		characterCapsule->GetOverlappingComponents(overlappings);
		for (auto& overlapPrim : overlappings)
		{
			if (overlapPrim == value.primitive.Get())
				return false;
		}
		value.primitive->SetCollisionResponseToChannel(ECC_Pawn, value.response);
		return true;
	});

	detectInteractableAround();

	if (pendingRelease)
		StopGrab();

	if (currentHoldingState == EHoldingState::LightGrabbing)
	{
		if (!holdingObject.IsValid())
		{
			UniversalRelease();
			return;
		}

		if (UPrimitiveComponent* prim = holdingObject->GetAssociatedComponent())
		{
			prim->SetWorldLocation(mainCharacter->GetHoldSocketLocation());
			prim->SetAllPhysicsLinearVelocity(FVector::ZeroVector);
			prim->SetAllPhysicsAngularVelocity(FVector::ZeroVector);
		}
	}
	else if (currentHoldingState == EHoldingState::Throwing || currentHoldingState == EHoldingState::ReleasingLightGrab)
	{
		if (UPrimitiveComponent* prim = holdingObject->GetAssociatedComponent())
		{
			FVector	middleTwoHands = mainCharacter->GetTwoHandsLocation();
			prim->SetWorldLocation(middleTwoHands);
		}
	}

	if (currentHoldingState == EHoldingState::HeavyGrabbing)
	{
		if (holdingObject.IsValid() && holdingObject->GetAssociatedComponent())
		{
			float currentDist = FVector::Dist(mainCharacter->GetActorLocation(), holdingObject->GetAssociatedComponent()->GetComponentLocation());
			if (FMath::Abs(currentDist - beginHeavyPushDistance) > 25.0f)
				StopGrab();
		}
	}

	if (currentHoldingState == EHoldingState::ReleasingLightGrab)
	{
		if (holdingObject.IsValid())
		{
			if (UPrimitiveComponent* holdingPrim = holdingObject->GetAssociatedComponent())
			{
				FHitResult	result;
				FCollisionQueryParams	params;
				params.AddIgnoredComponent(holdingPrim);
				params.AddIgnoredActor(GetOwner());
				FVector primExtent = holdingPrim->Bounds.BoxExtent;
				FVector start = holdingPrim->GetComponentLocation() - primExtent.Z;
				FVector endPoint = start + FVector::UpVector * -ReleasingDetectionValue;
				if (GetWorld()->SweepSingleByChannel(result, endPoint, endPoint, FQuat::Identity, 
					ECollisionChannel::ECC_Visibility, FCollisionShape::MakeBox(FVector(primExtent.Z * 0.5f, primExtent.Y * 0.5f, 0.2f)), params))
				{
					EndLightGrabRelease();
					mainCharacter->ReleaseMontageJumpPostSection();
				}
			}
		}
	}
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
	}
}

void	UHoldComponent::Grab()
{
	if (currentHoldingState == EHoldingState::LightGrabbing || currentHoldingState == EHoldingState::PreLightGrabbing)
		pendingRelease = false;
	if (currentHoldingState != EHoldingState::None || mainCharacter->IsInAir())
		return;
	if (!closestInteractableObject.IsValid() || !closestInteractableObject->IsGrabable())
		return;

	if (!closestInteractableObject->IsHeavy)
	{
		currentHoldingState = EHoldingState::PreLightGrabbing;
		holdingStateChangedDelegate.Broadcast(EHoldingState::None, EHoldingState::PreLightGrabbing);

		holdingObject = closestInteractableObject.Get();
		bool oneMeter = false;
		if (UPrimitiveComponent* prim = holdingObject->GetAssociatedComponent())
		{
			float diff = mainCharacter->GetActorLocation().Z - prim->GetComponentLocation().Z;
			if (diff < 15.0f)
				oneMeter = true;
		}
		mainCharacter->BlockCharacter();
		mainCharacter->PlayLightGrabMontage(oneMeter);
	}
	else
	{
		FRotator newRot = (closestInteractableNormal * -1.0f).Rotation();
		newRot.Roll = 0.0f;
		newRot.Pitch = 0.0f;

		FRotator tempRotator = (characterCapsule->GetComponentRotation() - newRot);
		tempRotator.Normalize();
		float deltaRot = FMath::Abs(tempRotator.Yaw);
		if (deltaRot > 35.0f)
			return;

		currentHoldingState = EHoldingState::PreHeavyGrabbing;
		holdingStateChangedDelegate.Broadcast(EHoldingState::None, EHoldingState::PreHeavyGrabbing);

		mainCharacter->BlockCharacter();

		holdingObject = closestInteractableObject.Get();

		FVector newLocation = closestInteractableLocation + closestInteractableNormal * HeavyGrabSnapDistance;
		newLocation.Z = mainCharacter->GetActorLocation().Z;

		beginHeavyPushDistance = FVector::Dist(newLocation, holdingObject->GetAssociatedComponent()->GetComponentLocation());

		FTimerDelegate	del;
		del.BindUFunction(mainCharacter, "PlayHeavyGrabMontage");

		mainCharacter->SnapCharacterTo(newLocation, newRot, 0.5f, del);
	}
}
void	UHoldComponent::BeginLightGrabPositionUpdate()
{
	if (!holdingObject.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Begin light grab position update : holding object is nullptr"));
		mainCharacter->UnblockCharacter();
		return;
	}

	if (!holdingObject->IsAssociatedValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Begin light grab position update : holding associated is nullptr"));
		mainCharacter->UnblockCharacter();
		return;
	}

	holdingObject->SetHoldComponent(this);
	holdingObject->onBeginGrab.Broadcast();
	if (holdingObject->IsSticked())
		holdingObject->Unstick();

	UPrimitiveComponent* tempPrim = holdingObject->GetAssociatedComponent();
	pendingReleasedPrimitives.RemoveAll([&](FPendingPrimitive& pendingPrim)
		{	return pendingPrim.primitive == tempPrim; });
	tempPrim->SetWorldRotation(characterCapsule->GetComponentRotation().Quaternion());	//RESET ROTATION
	tempPrim->SetCollisionProfileName("OverlapAllDynamic");
	previousGravityValue = tempPrim->IsGravityEnabled();
	tempPrim->SetEnableGravity(false);
	mainCharacter->SetHoldingObject(true);
	mainCharacter->UnblockCharacter();

	UAkGameplayStatics::PostEvent(holdingObject->TakeEvent, GetOwner());

	currentHoldingState = EHoldingState::LightGrabbing;
	holdingStateChangedDelegate.Broadcast(EHoldingState::PreLightGrabbing, EHoldingState::LightGrabbing);
}

void	UHoldComponent::EndHeavyGrab()
{
	currentHoldingState = EHoldingState::HeavyGrabbing;
	holdingStateChangedDelegate.Broadcast(EHoldingState::PreHeavyGrabbing, EHoldingState::HeavyGrabbing);

	//mainCharacter->EnableMovingHeavyObjectMode();
	mainCharacter->UnblockCharacter();
	//createHandConstraint();
	if (holdingObject.IsValid())
		holdingObject->SetHoldComponent(this);
}
	
void	UHoldComponent::EndLightGrabRelease()
{
	releaseLightGrabbedObject();
	mainCharacter->SetHoldingObject(false);
	mainCharacter->UnblockCharacter();

	//UAkGameplayStatics::PostEvent(holdingObject->HitEvent, GetOwner());
	currentHoldingState = EHoldingState::None;
	holdingStateChangedDelegate.Broadcast(EHoldingState::ReleasingLightGrab, EHoldingState::None);
}

void	UHoldComponent::StopGrab()
{
	if (currentHoldingState == EHoldingState::PreLightGrabbing)
	{
		mainCharacter->StopLightGrabMontage();
		mainCharacter->UnblockCharacter();
		holdingObject = nullptr;
		currentHoldingState = EHoldingState::None;
		holdingStateChangedDelegate.Broadcast(EHoldingState::PreLightGrabbing, EHoldingState::None);
	}
	else if (currentHoldingState == EHoldingState::PreHeavyGrabbing)
	{
		mainCharacter->StopSnapping();
		mainCharacter->StopHeavyGrabMontage();
		mainCharacter->UnblockCharacter();
		holdingObject = nullptr;
		currentHoldingState = EHoldingState::None;
		holdingStateChangedDelegate.Broadcast(EHoldingState::PreHeavyGrabbing, EHoldingState::None);
	}
	else if (currentHoldingState == EHoldingState::LightGrabbing)
	{
		if (!checkReleasing())
		{
			pendingRelease = true;
			return;
		}
		pendingRelease = false;
		mainCharacter->PlayLightGrabReleaseMontage();
		mainCharacter->BlockCharacter();
		currentHoldingState = EHoldingState::ReleasingLightGrab;
		holdingStateChangedDelegate.Broadcast(EHoldingState::LightGrabbing, EHoldingState::ReleasingLightGrab);
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
		mainCharacter->PlayLightThrowMontage();
	}
	else if (currentHoldingState == EHoldingState::HeavyGrabbing)
	{
		currentHoldingState = EHoldingState::HeavyThrowing;
		holdingStateChangedDelegate.Broadcast(EHoldingState::HeavyGrabbing, EHoldingState::Throwing);
		mainCharacter->PlayHeavyThrowMontage();
	}
}

void	UHoldComponent::EndThrow()
{
	if (currentHoldingState == EHoldingState::Throwing)
	{
		UInteractableComponent* tempInter = holdingObject.IsValid() ? holdingObject.Get() : nullptr;
		UInteractableComponent* tempInteractablePrim = holdingObject.IsValid() ? holdingObject.Get() : nullptr;
		releaseLightGrabbedObject();
		if (tempInter)
			tempInter->SetThrown();
		if (tempInteractablePrim)
		{
			FRotator	tempRotation = characterCapsule->GetComponentRotation();
			tempRotation.Pitch += AdditionalThrowAngle;
			tempInteractablePrim->GetAssociatedComponent()->AddImpulse(tempRotation.Vector() * ThrowPower);
		}

		mainCharacter->SetHoldingObject(false);
		holdingStateChangedDelegate.Broadcast(EHoldingState::Throwing, EHoldingState::None);
		currentHoldingState = EHoldingState::None;
	}
	else if (currentHoldingState == EHoldingState::HeavyThrowing)
	{
		releaseHeavyGrabbedObject();
		
		if (holdingObject.IsValid() && holdingObject->IsAssociatedValid())
			holdingObject->GetAssociatedComponent()->AddImpulse(characterCapsule->GetForwardVector() * HeavyThrowPower);
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

		// SNAP HOOK
		FRotator newRot = (closestInteractableNormal * -1.0f).Rotation();
		newRot.Roll = 0.0f;
		newRot.Pitch = 0.0f;

		FRotator tempRotator = (holdingObject->GetOwner()->GetActorRotation() - newRot);
		tempRotator.Normalize();
		float deltaRot = FMath::Abs(tempRotator.Yaw);

		holdingObject->GetAssociatedComponent()->SetWorldRotation(newRot);

		FVector	ClosestPoint;
		closestInteractableObject->GetAssociatedComponent()->GetClosestPointOnCollision(holdingObject->GetAssociatedComponent()->GetComponentLocation(), ClosestPoint);
		holdingObject->GetAssociatedComponent()->SetWorldLocation(ClosestPoint - holdingObject->GetAssociatedComponent()->GetForwardVector() * 50.0f);

		// SNAP CHARACTER
		tempRotator = (characterCapsule->GetComponentRotation() - newRot);
		tempRotator.Normalize();
		deltaRot = FMath::Abs(tempRotator.Yaw);

		mainCharacter->SetActorLocation(ClosestPoint - holdingObject->GetAssociatedComponent()->GetForwardVector() * 50.0f - characterCapsule->GetForwardVector() * 50.0f);
		mainCharacter->SetActorRotation(newRot);

		closestInteractableObject->AddStickConstraint(holdingObject.Get(), holdingObject->GetAssociatedComponent(), TEXT("None"));
		releaseLightGrabbedObject();
		holdingStateChangedDelegate.Broadcast(EHoldingState::LightGrabbing, EHoldingState::Sticking);

		mainCharacter->UnblockCharacter();
		mainCharacter->SetHoldingObject(false);
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

void	UHoldComponent::CancelThrow()
{
	releaseLightGrabbedObject();
	mainCharacter->SetHoldingObject(false);
	currentHoldingState = EHoldingState::None;
	mainCharacter->SetThrowingObject(false);
}

void	UHoldComponent::CancelHeavyThrow()
{
	releaseHeavyGrabbedObject();
	mainCharacter->DisableMovingHeavyObjectMode();
	mainCharacter->SetThrowingObject(false);
	currentHoldingState = EHoldingState::None;
}

void	UHoldComponent::CancelLightGrab()
{
	releaseLightGrabbedObject();
	mainCharacter->SetHoldingObject(false);
	mainCharacter->UnblockCharacter();
	currentHoldingState = EHoldingState::None;
}

void	UHoldComponent::CancelHeavyGrab()
{
	mainCharacter->DisableMovingHeavyObjectMode();
	mainCharacter->UnblockCharacter();
	holdingObject = nullptr;
	currentHoldingState = EHoldingState::None;
}

void	UHoldComponent::releaseLightGrabbedObject()
{
	if (holdingObject.IsValid())
	{
		if (holdingObject->IsAssociatedValid())
		{
			holdingObject->GetAssociatedComponent()->SetCollisionProfileName("SmallInteractable");
			holdingObject->GetAssociatedComponent()->SetEnableGravity(previousGravityValue);
			FPendingPrimitive	pendingPrim;
			pendingPrim.primitive = holdingObject->GetAssociatedComponent();
			pendingPrim.response = pendingPrim.primitive->GetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn);
			pendingReleasedPrimitives.Add(pendingPrim);
			pendingPrim.primitive->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		}
		holdingObject->SetHoldComponent(nullptr);
		holdingObject->onEndGrab.Broadcast();
	}
	holdingObject = nullptr;
}

void	UHoldComponent::releaseHeavyGrabbedObject()
{
	if (holdingObject.IsValid())
		holdingObject->SetHoldComponent(nullptr);
	holdingObject = nullptr;
}

void	UHoldComponent::detectInteractableAround()
{
	float closestInteractable = FLT_MAX;

	TArray<FHitResult>			hitResults;
	FCollisionQueryParams		queryParams;
	queryParams.AddIgnoredActor(GetOwner());
	if (holdingObject.IsValid())
		queryParams.AddIgnoredActor(holdingObject->GetOwner());

	/*
	FVector	beginTrace = characterCapsule->GetComponentLocation();
	FVector traceLocation = characterCapsule->GetComponentLocation() + GetOwner()->GetActorForwardVector() * 2.0f * characterCapsule->GetScaledCapsuleRadius();
	GetWorld()->SweepMultiByChannel(hitResults, beginTrace, traceLocation, characterCapsule->GetComponentQuat(), ECollisionChannel::ECC_GameTraceChannel2,
		FCollisionShape::MakeBox(FVector(characterCapsule->GetScaledCapsuleRadius(), characterCapsule->GetScaledCapsuleRadius(), 
			characterCapsule->GetScaledCapsuleHalfHeight())), queryParams);
	*/

	FVector	beginTrace = characterCapsule->GetComponentLocation() + characterCapsule->GetScaledCapsuleRadius() * 0.9f * characterCapsule->GetForwardVector();
	FVector	endTrace = characterCapsule->GetComponentLocation() + characterCapsule->GetScaledCapsuleRadius() * 2.0f * characterCapsule->GetForwardVector();

	GetWorld()->SweepMultiByChannel(hitResults, beginTrace, endTrace, characterCapsule->GetComponentQuat(), ECollisionChannel::ECC_GameTraceChannel2, 
		FCollisionShape::MakeCapsule(characterCapsule->GetScaledCapsuleRadius(), characterCapsule->GetScaledCapsuleHalfHeight()), queryParams);

	if (hitResults.Num() > 0)
	{
		for (auto& hitRes : hitResults)
		{
			if (!hitRes.Component.IsValid())
				continue;

			float scalRes = FVector::DotProduct(characterCapsule->GetForwardVector(), hitRes.ImpactPoint - characterCapsule->GetComponentLocation());
			if (scalRes < 0.0f)
				continue;

			UInteractableComponent* interComp = UInteractableComponent::FindAssociatedInteractableComponent(hitRes.Component.Get());
			if (interComp == holdingObject)
			{
				closestInteractableLocation = hitRes.ImpactPoint;
				closestInteractableNormal = hitRes.Normal;
				continue;
			}
			if (interComp)
			{
				float distance = (hitRes.Component->GetComponentLocation() - characterCapsule->GetComponentLocation()).Size();
				if (distance < closestInteractable)
				{
					closestInteractable = distance;
					closestInteractableObject = interComp;
					closestInteractableLocation = hitRes.ImpactPoint;
					closestInteractableNormal = hitRes.Normal;
				}
			}
		}
	}
}

bool	UHoldComponent::checkReleasing()
{
	if (holdingObject.IsValid())
	{
		if (UPrimitiveComponent* holdingPrim = holdingObject->GetAssociatedComponent())
		{
			FHitResult	hitResult;
			FCollisionQueryParams	params;
			params.AddIgnoredComponent(holdingPrim);
			params.AddIgnoredActor(GetOwner());
			FVector primExtent = holdingPrim->Bounds.BoxExtent;
			FVector start = holdingPrim->GetComponentLocation();

			return !GetWorld()->SweepSingleByChannel(hitResult, start, start, FQuat::Identity, ECollisionChannel::ECC_Visibility, FCollisionShape::MakeBox(primExtent), params);
		}
	}
	return false;
}