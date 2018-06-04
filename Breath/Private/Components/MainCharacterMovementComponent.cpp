// Fill out your copyright notice in the Description page of Project Settings.

#include "MainCharacterMovementComponent.h"

#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "PhysicsEngine/BodySetup.h"

#include "Runtime/Engine/Classes/GameFramework/Actor.h"

#include "InteractableComponent.h"

void UMainCharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	FVector last = LastUpdateLocation;
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	lastOffsetLocation = LastUpdateLocation - last;
	
	currentPushAndPullSpeed = FMath::FInterpTo(currentPushAndPullSpeed, pushAndPullTargetSpeed, DeltaTime, PushPullInterpSpeed);
	currentRotateSpeed = FMath::FInterpTo(currentRotateSpeed, rotateTargetSpeed, DeltaTime, RotateInterpSpeed);

	if (!FMath::IsNearlyZero(currentPushAndPullSpeed))
		updatePushAndPull();
	if (!FMath::IsNearlyZero(currentRotateSpeed))
		updateRotatePushAndPull();
}

/*
bool UMainCharacterMovementComponent::CheckFall(const FFindFloorResult& OldFloor, const FHitResult& Hit, const FVector& Delta, const FVector& OldLocation, float remainingTime, float timeTick, int32 Iterations, bool bMustJump)
{
	if (!HasValidData())
	{
		return false;
	}

	if (bMustJump || CanWalkOffLedges())
	{
		CharacterOwner->OnWalkingOffLedge(OldFloor.HitResult.ImpactNormal, OldFloor.HitResult.Normal, OldLocation, timeTick);
		if (IsMovingOnGround())
		{
			currentCoyoteTime += timeTick;
			if (currentCoyoteTime >= CoyoteTime)
			{
				// If still walking, then fall. If not, assume the user set a different mode they want to keep.
				StartFalling(Iterations, remainingTime, timeTick, Delta, OldLocation);
				currentCoyoteTime = 0.0f;
			}
		}
		return true;
	}
	return false;
}
*/

bool UMainCharacterMovementComponent::DoJump(bool bReplayingMoves)
{
	//return Super::DoJump(bReplayingMoves);
	//ALL OF THIS IS SUPER::DOJUMP
	if (CharacterOwner && CharacterOwner->CanJump())
	{
		// Don't jump if we can't move up/down.
		if (!bConstrainToPlane || FMath::Abs(PlaneConstraintNormal.Z) != 1.f)
		{
			Velocity.Z = JumpZVelocity;
			SetMovementMode(MOVE_Falling);
			bIsJumping = true;				//EXCEPT THIS LINE (IMPRTANT FOR ANIMATIONS)
			bCanWalkOffLedges = true;		//AND THIS ONE
			return true;
		}
	}

	return false;
	/*if (CharacterOwner && CharacterOwner->CanJump())
	{
		if (!bConstrainToPlane || FMath::Abs(PlaneConstraintNormal.Z) != 1.f)
		{
			Velocity += FVector::UpVector * JumpZVelocity + jumpDirection * LateralJumpForce;
			SetMovementMode(MOVE_Falling);
			jumpDirection = FVector::ZeroVector;
			bOrientRotationToMovement = false;
			return true;
		}
	}
	return false;*/
}
	
void UMainCharacterMovementComponent::SetPostLandedPhysics(const FHitResult& Hit)
{
	Super::SetPostLandedPhysics(Hit);

	EndJumping();
	if (MovementMode == EMovementMode::MOVE_Walking)
		bOrientRotationToMovement = true;
}
	
void	UMainCharacterMovementComponent::ProcessPushAndPull(float const& coeff, UInteractableComponent* holdingObj)
{
	holdingObject = holdingObj;
	if (holdingObject == nullptr || holdingObject->GetOwner() == nullptr)
		pushAndPullTargetSpeed = 0.0f;
	pushAndPullTargetSpeed = coeff * PushPullSpeed;
}

void	UMainCharacterMovementComponent::ProcessRotateHeavyObject(float direction, UInteractableComponent* holdingObj, FVector holdingObjectLocation)
{
	holdingObject = holdingObj;
	if (holdingObject == nullptr || holdingObject->GetOwner() == nullptr)
		rotateTargetSpeed = 0.0f;
	if (direction == -1.0f && holdingObject->bLockLeftRotate)
		rotateTargetSpeed = 0.0f;
	if (direction == 1.0f && holdingObject->bLockRightRotate)
		rotateTargetSpeed = 0.0f;
	rotateTargetSpeed = direction * RotationSpeed;
}

void	UMainCharacterMovementComponent::ProcessThrowRotation(float coeff)
{
	FRotator characterRot = GetCharacterOwner()->GetActorRotation();
	characterRot.Yaw += ThrowRotationSpeed * coeff;
	GetCharacterOwner()->SetActorRotation(characterRot);
}

void	UMainCharacterMovementComponent::EndJumping()
{
	bIsJumping = false;
	bCanWalkOffLedges = true;
}

void	UMainCharacterMovementComponent::SetWalkMode()
{
	if (currentLocomotionState == ELocomotionState::Walking)
		return;
	MaxWalkSpeed = WalkSpeed;
	MinAnalogWalkSpeed = WalkSpeed;
	bCanWalkOffLedges = false;
	onLocomotionStateChanged.Broadcast(currentLocomotionState, ELocomotionState::Walking);
	currentLocomotionState = ELocomotionState::Walking;
}

void	UMainCharacterMovementComponent::SetJogMode() 
{
	if (currentLocomotionState == ELocomotionState::Running)
		return;
	MaxWalkSpeed = JogSpeed;
	MinAnalogWalkSpeed = JogSpeed;
	bCanWalkOffLedges = true;
	onLocomotionStateChanged.Broadcast(currentLocomotionState, ELocomotionState::Running);
	currentLocomotionState = ELocomotionState::Running;
}
	
void	UMainCharacterMovementComponent::SetCustomSpeed(float customSpeed)
{
	MaxWalkSpeed = customSpeed;
	MinAnalogWalkSpeed = customSpeed;
	onLocomotionStateChanged.Broadcast(currentLocomotionState, ELocomotionState::Custom);
	currentLocomotionState = ELocomotionState::Custom;
}

bool	UMainCharacterMovementComponent::IsFalling(bool& ascending)
{
	ascending = Velocity.Z >= 0.0f;
	return UCharacterMovementComponent::IsFalling();
}

void	UMainCharacterMovementComponent::updatePushAndPull()
{
	bool pushing = currentPushAndPullSpeed >= 0.0f;

	FVector MoveDir = GetCharacterOwner()->GetActorRotation().Vector() * (pushing? 1.0f : -1.0f);

	FHitResult SweepResult;

	// 	holdingObject->GetOwner()->AddActorWorldOffset(FVector(0.f, 0.f, 1.f) * 10.f, true);
	// 	holdingObject->GetAssociatedComponent()->AddForce(MoveDir * 1000.f);//this->PushPullSpeed);
	// 	this->GetOwner()->AddActorWorldOffset(MoveDir * PushPullSpeed * GetWorld()->GetDeltaSeconds(), true, &SweepResult);

	if (!pushing)
	{
		if (holdingObject->bLockPull)	return;
		this->GetOwner()->AddActorWorldOffset(MoveDir * -currentPushAndPullSpeed * GetWorld()->GetDeltaSeconds(), true, &SweepResult);

		if (SweepResult.bBlockingHit == false)
		{
			// 			TArray<FHitResult> OutHits;
			// 			FCollisionQueryParams CollisionQueryParams;
			// 			CollisionQueryParams.AddIgnoredActor(this->GetOwner());
			// 			CollisionQueryParams.AddIgnoredActor(holdingObject->GetOwner());
			// 
			// 			const TArray<UInteractableComponent::FStickConstraint>& StickConstraints = holdingObject->GetStickConstraints();
			// 
			// 			for (auto StickConstraint : StickConstraints)
			// 			{
			// 				CollisionQueryParams.AddIgnoredActor(StickConstraint.hook->GetOwner());
			// 				CollisionQueryParams.AddIgnoredActor(StickConstraint.hook->GetOwner()->GetRootComponent()->GetAttachParent()->GetOwner());
			// 			}
			// 
			// 			FVector EndLocation = MoveDir * PushPullSpeed * GetWorld()->GetDeltaSeconds();
			// 
			// 			GetWorld()->SweepMultiByChannel(
			// 				OutHits,
			// 				holdingObject->GetOwner()->GetActorLocation() + FVector(0.f, 0.f, 50.f),
			// 				holdingObject->GetOwner()->GetActorLocation() + EndLocation + FVector(0.f, 0.f, 50.f),
			// 				holdingObject->GetOwner()->GetActorQuat(),
			// 				ECC_WorldDynamic,
			// 				FCollisionShape::MakeBox(holdingObject->GetOwner()->GetComponentsBoundingBox(false).GetExtent() * 0.9f),
			// 				CollisionQueryParams
			// 			);

			holdingObject->GetOwner()->AddActorWorldOffset(FVector(0.f, 0.f, 1.f) * 10.f, true);
			holdingObject->GetOwner()->AddActorWorldOffset(MoveDir * -currentPushAndPullSpeed * GetWorld()->GetDeltaSeconds(), true, &SweepResult);


			// 			for (auto Hit : OutHits)
			// 			{
			// 				if (Hit.bBlockingHit == true)
			// 				{
			// 					this->GetOwner()->AddActorWorldOffset(-MoveDir * PushPullSpeed * GetWorld()->GetDeltaSeconds(), true, &SweepResult);
			// 					return;
			// 				}
			// 			}
			// 
			// 			holdingObject->GetOwner()->AddActorWorldOffset(EndLocation);

			if (SweepResult.bBlockingHit == true)
			{
				this->GetOwner()->AddActorWorldOffset(-MoveDir * -currentPushAndPullSpeed * GetWorld()->GetDeltaSeconds(), true, &SweepResult);
			}

			holdingObject->GetOwner()->AddActorWorldOffset(FVector(0.f, 0.f, 1.f) * -10.f, true);

		}
	}
	else
	{
		if (holdingObject->bLockPush)	return;
		// 		TArray<FHitResult> OutHits;
		// 		FCollisionQueryParams CollisionQueryParams;
		// 		CollisionQueryParams.AddIgnoredActor(this->GetOwner());
		// 		CollisionQueryParams.AddIgnoredActor(holdingObject->GetOwner());
		// 
		// 		const TArray<UInteractableComponent::FStickConstraint>& StickConstraints = holdingObject->GetStickConstraints();
		// 
		// 		for (auto StickConstraint : StickConstraints)
		// 		{
		// 			CollisionQueryParams.AddIgnoredActor(StickConstraint.hook->GetOwner());
		// 			CollisionQueryParams.AddIgnoredActor(StickConstraint.hook->GetOwner()->GetRootComponent()->GetAttachParent()->GetOwner());
		// 		}
		// 
		// 		FVector EndLocation = MoveDir * PushPullSpeed * GetWorld()->GetDeltaSeconds();
		// 
		// 		GetWorld()->SweepMultiByChannel(
		// 			OutHits,
		// 			holdingObject->GetOwner()->GetActorLocation() + FVector(0.f, 0.f, 50.f),
		// 			holdingObject->GetOwner()->GetActorLocation() + EndLocation + FVector(0.f, 0.f, 50.f),
		// 			holdingObject->GetOwner()->GetActorQuat(),
		// 			ECC_WorldDynamic,
		// 			FCollisionShape::MakeBox(holdingObject->GetOwner()->GetComponentsBoundingBox(false).GetExtent() * 0.9f),
		// 			CollisionQueryParams
		// 		);
		// 
		// 		for (auto Hit : OutHits)
		// 		{
		// 			if (Hit.bBlockingHit == true)
		// 			{
		// 				return;
		// 			}
		// 		}
		// 
		// 		this->GetOwner()->AddActorWorldOffset(MoveDir * PushPullSpeed * GetWorld()->GetDeltaSeconds(), true, &SweepResult);
		// 	
		// 		if (SweepResult.bBlockingHit == false)
		// 		{
		// 			holdingObject->GetOwner()->AddActorWorldOffset(EndLocation);
		// 		}

		holdingObject->GetOwner()->AddActorWorldOffset(FVector(0.f, 0.f, 1.f) * 10.f, true);
		holdingObject->GetOwner()->AddActorWorldOffset(MoveDir * currentPushAndPullSpeed * GetWorld()->GetDeltaSeconds(), true, &SweepResult);

		if (SweepResult.bBlockingHit == false)
		{
			this->GetOwner()->AddActorWorldOffset(MoveDir * currentPushAndPullSpeed * GetWorld()->GetDeltaSeconds(), true, &SweepResult);

			if (SweepResult.bBlockingHit == true)
			{
				holdingObject->GetOwner()->AddActorWorldOffset(-MoveDir * currentPushAndPullSpeed * GetWorld()->GetDeltaSeconds(), true, &SweepResult);
			}
		}

		holdingObject->GetOwner()->AddActorWorldOffset(FVector(0.f, 0.f, 1.f) * -10.f, true);
	}
}

void	UMainCharacterMovementComponent::updateRotatePushAndPull()
{
	if (!holdingObject)
		return;
	float angle = -currentRotateSpeed * GetWorld()->GetDeltaSeconds();
	//MAYBE USE OBJECT WEIGHT
	//TO DO REPLACE THIS OLD WAY OF ROTATING OBJECT
	FRotator	rotation = FQuat(FVector::UpVector, FMath::DegreesToRadians(angle)).Rotator();
	FVector		holdingToCharac = GetActorLocation() - holdingObject->GetAssociatedLocation();
	FVector		holdingToNewLoc = rotation.RotateVector(holdingToCharac);
	FVector		finalLocation = holdingToNewLoc + holdingObject->GetAssociatedLocation();
	FRotator	finalRotation = rotation + GetCharacterOwner()->GetActorRotation();
	FRotator	finalObjectRotation = rotation + holdingObject->GetOwner()->GetActorRotation();

	FHitResult SweepResult;
	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(GetCharacterOwner());
	//CollisionQueryParams.bTraceComplex = true;
	//CollisionQueryParams.bIgnoreTouches = true;

	GetWorld()->SweepSingleByChannel(
		SweepResult,
		GetCharacterOwner()->GetActorLocation(),
		finalLocation,
		FQuat::Identity,
		ECC_WorldDynamic,
		GetCharacterOwner()->GetCapsuleComponent()->GetCollisionShape(),
		CollisionQueryParams
	);

	GetCharacterOwner()->SetActorLocation(finalLocation, true, &SweepResult);

	if (SweepResult.bBlockingHit == false)
	{
		CollisionQueryParams = FCollisionQueryParams::DefaultQueryParam;
		CollisionQueryParams.AddIgnoredActor(holdingObject->GetOwner());

		FVector	objectFinalLocationWithZOffset = holdingObject->GetOwner()->GetActorLocation();
		objectFinalLocationWithZOffset.Z += 10.f;

		holdingObject->GetOwner()->SetActorLocationAndRotation(holdingObject->GetOwner()->GetActorLocation(), finalObjectRotation.Quaternion(), true, &SweepResult);

		if (SweepResult.bBlockingHit == true)
		{
			holdingToCharac = GetActorLocation() - holdingObject->GetAssociatedLocation();
			holdingToNewLoc = rotation.RotateVector(holdingToCharac);
			finalLocation = holdingToNewLoc + holdingObject->GetAssociatedLocation();
			holdingObject->GetOwner()->SetActorLocationAndRotation(holdingObject->GetOwner()->GetActorLocation(), finalObjectRotation.Quaternion(), true);
		}

		GetCharacterOwner()->SetActorLocation(finalLocation, true);
	}

	FVector LookAtDir = holdingObject->GetOwner()->GetActorLocation() - GetCharacterOwner()->GetActorLocation();
	LookAtDir.Normalize();

	FRotator LookAtOnZ = FRotationMatrix::MakeFromX(LookAtDir).Rotator();
	LookAtOnZ.Pitch = 0.f;
	LookAtOnZ.Roll = 0.f;

	GetCharacterOwner()->SetActorRotation(LookAtOnZ);
}

void UMainCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	if (PreviousMovementMode == EMovementMode::MOVE_Falling)
	{
		this->RotationRate.Yaw = this->MoveRotationSpeed;
	}
	if (this->MovementMode == EMovementMode::MOVE_Falling)
	{
		this->RotationRate.Yaw = this->FallingRotationSpeed;
	}
}
