// Fill out your copyright notice in the Description page of Project Settings.

#include "MainCharacterMovementComponent.h"

#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "PhysicsEngine/BodySetup.h"

#include "InteractableComponent.h"

void UMainCharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	FVector last = LastUpdateLocation;
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	lastOffsetLocation = LastUpdateLocation - last;
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
	
void	UMainCharacterMovementComponent::ProcessPushAndPull(float const& coeff, UInteractableComponent* holdingObject)
{
	FVector MoveDir = GetCharacterOwner()->GetActorRotation().Vector() * coeff;

	FHitResult SweepResult;

// 	holdingObject->GetOwner()->AddActorWorldOffset(FVector(0.f, 0.f, 1.f) * 10.f, true);
// 	holdingObject->GetAssociatedComponent()->AddForce(MoveDir * 1000.f);//this->PushPullSpeed);
// 	this->GetOwner()->AddActorWorldOffset(MoveDir * PushPullSpeed * GetWorld()->GetDeltaSeconds(), true, &SweepResult);

	if (coeff < 0.f)
	{
		this->GetOwner()->AddActorWorldOffset(MoveDir * PushPullSpeed * GetWorld()->GetDeltaSeconds(), true, &SweepResult);

		if (SweepResult.bBlockingHit == false)
		{
			holdingObject->GetOwner()->AddActorWorldOffset(FVector(0.f, 0.f, 1.f) * 10.f, true);
			holdingObject->GetOwner()->AddActorWorldOffset(MoveDir * PushPullSpeed * GetWorld()->GetDeltaSeconds(), true, &SweepResult);

			if (SweepResult.bBlockingHit == true)
			{
				this->GetOwner()->AddActorWorldOffset(-MoveDir * PushPullSpeed * GetWorld()->GetDeltaSeconds(), true, &SweepResult);
			}

			holdingObject->GetOwner()->AddActorWorldOffset(FVector(0.f, 0.f, 1.f) * -10.f, true);

		}
	}
	else
	{
		holdingObject->GetOwner()->AddActorWorldOffset(FVector(0.f, 0.f, 1.f) * 10.f, true);
		holdingObject->GetOwner()->AddActorWorldOffset(MoveDir * PushPullSpeed * GetWorld()->GetDeltaSeconds(), true, &SweepResult);

		if (SweepResult.bBlockingHit == false)
		{
			this->GetOwner()->AddActorWorldOffset(MoveDir * PushPullSpeed * GetWorld()->GetDeltaSeconds(), true, &SweepResult);

			if (SweepResult.bBlockingHit == true)
			{
				holdingObject->GetOwner()->AddActorWorldOffset(-MoveDir * PushPullSpeed * GetWorld()->GetDeltaSeconds(), true, &SweepResult);
			}
		}

		holdingObject->GetOwner()->AddActorWorldOffset(FVector(0.f, 0.f, 1.f) * -10.f, true);
	}
}

void	UMainCharacterMovementComponent::ProcessRotateHeavyObject(bool direction, UInteractableComponent* holdingObject, FVector holdingObjectLocation)
{
	float angle = RotationSpeed * GetWorld()->GetDeltaSeconds();
	angle *= direction ? -1.0f : 1.0f;
	//MAYBE USE OBJECT WEIGHT
	//TO DO REPLACE THIS OLD WAY OF ROTATING OBJECT
	FRotator	rotation = FQuat(FVector::UpVector, FMath::DegreesToRadians(angle)).Rotator();
	FVector		holdingToCharac = GetActorLocation() - holdingObjectLocation;
	FVector		holdingToNewLoc = rotation.RotateVector(holdingToCharac);
	FVector		finalLocation = holdingToNewLoc + holdingObjectLocation;
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
			holdingToCharac = GetActorLocation() - holdingObjectLocation;
			holdingToNewLoc = rotation.RotateVector(holdingToCharac);
			finalLocation = holdingToNewLoc + holdingObjectLocation;
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
	MaxWalkSpeed = WalkSpeed;
	MinAnalogWalkSpeed = WalkSpeed;
	bCanWalkOffLedges = false;
}

void	UMainCharacterMovementComponent::SetJogMode() 
{
	MaxWalkSpeed = JogSpeed;
	MinAnalogWalkSpeed = JogSpeed;
	bCanWalkOffLedges = true;
}
	
void	UMainCharacterMovementComponent::SetCustomSpeed(float customSpeed)
{
	MaxWalkSpeed = customSpeed;
	MinAnalogWalkSpeed = customSpeed;
}

bool	UMainCharacterMovementComponent::IsFalling(bool& ascending)
{
	ascending = Velocity.Z >= 0.0f;
	return UCharacterMovementComponent::IsFalling();
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
