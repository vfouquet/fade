// Fill out your copyright notice in the Description page of Project Settings.

#include "MainCharacterMovementComponent.h"

#include "GameFramework/Character.h"

void	UMainCharacterMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

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
	
FVector UMainCharacterMovementComponent::GetAirControl(float DeltaTime, float TickAirControl, const FVector& FallAcceleration)
{
	FVector	fallDir;
	float	fallAccel;
	FallAcceleration.ToDirectionAndLength(fallDir, fallAccel);
	return  fallDir * AirControlPower;
}
	
void	UMainCharacterMovementComponent::ProcessRotateHeavyObject(bool direction, float holdingObjectMass)
{
	float	massMult = 0.0f;
	if (holdingObjectMass <= MassGrabValues[0])
		massMult = MassGrabMultipliers[0];
	else if (holdingObjectMass <= MassGrabValues[1])
		massMult = MassGrabMultipliers[1];
	else if (holdingObjectMass <= MassGrabValues[2])
		massMult = MassGrabMultipliers[2];

	FVector MoveDir = GetCharacterOwner()->GetActorRotation().Vector();
	AddInputVector(MoveDir * massMult * (direction ? 1.0f : -1.0f));
}

void	UMainCharacterMovementComponent::ProcessPushPull(bool direction, float holdingObjectMass, FVector holdingObjectLocation)
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