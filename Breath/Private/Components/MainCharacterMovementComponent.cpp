// Fill out your copyright notice in the Description page of Project Settings.

#include "MainCharacterMovementComponent.h"

#include "GameFramework/Character.h"
	
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
			//PerchRadiusThreshold = 0.0f;	//AND THIS ONE TOO
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
	
void	UMainCharacterMovementComponent::ProcessPushAndPull(float const& coeff, float holdingObjectMass)
{
	float	massMult = 0.0f;
	if (holdingObjectMass <= MassGrabValues[0])
		massMult = MassGrabMultipliers[0];
	else if (holdingObjectMass <= MassGrabValues[1])
		massMult = MassGrabMultipliers[1];
	else if (holdingObjectMass <= MassGrabValues[2])
		massMult = MassGrabMultipliers[2];

	FVector MoveDir = GetCharacterOwner()->GetActorRotation().Vector();
	AddInputVector(MoveDir * massMult * coeff);
}

void	UMainCharacterMovementComponent::ProcessRotateHeavyObject(bool direction, float holdingObjectMass, FVector holdingObjectLocation)
{
	float	massMult = 0.0f;
	if (holdingObjectMass <= MassGrabValues[0])
		massMult = MassGrabMultipliers[0];
	else if (holdingObjectMass <= MassGrabValues[1])
		massMult = MassGrabMultipliers[1];
	else if (holdingObjectMass <= MassGrabValues[2])
		massMult = MassGrabMultipliers[2];
	float angle = RotationSpeed * GetWorld()->GetDeltaSeconds() * massMult;
	angle *= direction ? -1.0f : 1.0f;
	//MAYBE USE OBJECT WEIGHT
	//TO DO REPLACE THIS OLD WAY OF ROTATING OBJECT
	FRotator	rotation = FQuat(FVector::UpVector, FMath::DegreesToRadians(angle)).Rotator();
	FVector		holdingToCharac = GetActorLocation() - holdingObjectLocation;
	FVector		holdingToNewLoc = rotation.RotateVector(holdingToCharac);
	FVector		finalLocation = holdingToNewLoc + holdingObjectLocation;
	FRotator	finalRotation = rotation + GetCharacterOwner()->GetActorRotation();
	GetCharacterOwner()->SetActorLocationAndRotation(finalLocation, finalRotation.Quaternion(), true, nullptr, ETeleportType::None);
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
	bCanWalkOffLedges = true;// Velocity.Size() > WalkSpeed ? true : false;
	//PerchRadiusThreshold = 17.0f;
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
