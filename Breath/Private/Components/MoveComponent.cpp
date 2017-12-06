// Fill out your copyright notice in the Description page of Project Settings.

#include "MoveComponent.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values for this component's properties
UMoveComponent::UMoveComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UMoveComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UMoveComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UMoveComponent::MoveForward(float Value)
{
	if (isBlocked)
		return;

	ACharacter* Char = Cast<ACharacter>(GetOwner());
	APlayerController* CharacterController = Cast<APlayerController>(Char->GetController());

	float	realValue = 0.0f;

	if (isMovingHeavyObject)
	{
		currentInputValue.Y = Value;
		if (Value == 0.0f)
			return;

		float	inputAngle = GetInputAngle();
		float	cameraDiffAngle = GetCameraTargetDiffAngle();

		float difference = inputAngle - cameraDiffAngle;

		if (difference >= -HeavyAngleTolerance && difference <= HeavyAngleTolerance)
			realValue = 1.0f;
		else if (difference >= -180.0f - HeavyAngleTolerance && difference <= -180.0f + HeavyAngleTolerance)
			realValue = -1.0f;
		else
		{
			float coeff = GetRotateMultiplayer(difference) * RotationSpeed * GetWorld()->GetDeltaSeconds();
			//MAYBE USE DELTA TIME AND OBJECT WEIGHT
			FRotator	rotation = FQuat(FVector::UpVector, FMath::DegreesToRadians(coeff)).Rotator();
			FVector		holdingToCharac = Char->GetActorLocation() - holdingObjectLocation;
			FVector		holdingToNewLoc = rotation.RotateVector(holdingToCharac);
			FVector		finalLocation = holdingToNewLoc + holdingObjectLocation;
			FRotator	finalRotation = rotation + Char->GetActorRotation();
			Char->SetActorLocationAndRotation(finalLocation, finalRotation.Quaternion(), true, nullptr, ETeleportType::TeleportPhysics);
			return;
		}
	}
	else
	{
		bool neg = false;
		if (Value < 0.0f)
		{
			neg = true;
			Value *= -1.0f;
		}

		if (Value <= WalkThreshold)
			return;
		realValue = (Value <= JogThreshold) ? JogThreshold : 1.0f;
		realValue *= (neg) ? -1.0f : 1.0f;
	}

	FRotator CamRot = CharacterController->PlayerCameraManager->GetCameraRotation();
	CamRot.Pitch = 0.0f;
	FVector MoveDir = CamRot.Vector();
	Char->GetCharacterMovement()->AddInputVector(MoveDir * realValue);
}

void UMoveComponent::MoveRight(float Value)
{
	if (isBlocked)
		return;

	ACharacter* Char = Cast<ACharacter>(GetOwner());
	APlayerController* CharacterController = Cast<APlayerController>(Char->GetController());

	if (!isMovingHeavyObject)
	{
		bool neg = false;
		if (Value < 0.0f)
		{
			neg = true;
			Value *= -1.0f;
		}

		if (Value <= WalkThreshold)
			return;

		float realValue = (Value <= JogThreshold) ? JogThreshold : 1.0f;
		realValue *= (neg) ? -1.0f : 1.0f;

		FRotator CamRot = CharacterController->PlayerCameraManager->GetCameraRotation();
		CamRot.Pitch = 0.0f;
		FVector MoveDir = CamRot.RotateVector(FVector::RightVector);
		Char->GetCharacterMovement()->AddInputVector(MoveDir * realValue);
	}
	else
	{
		currentInputValue.X = Value;
		if (Value == 0.0f)
			return;

		float	inputAngle = GetInputAngle();
		float	cameraDiffAngle = GetCameraTargetDiffAngle();

		float difference = inputAngle - cameraDiffAngle;

		if (difference >= -HeavyAngleTolerance && difference <= HeavyAngleTolerance)
		{
			FRotator CamRot = CharacterController->PlayerCameraManager->GetCameraRotation();
			CamRot.Pitch = 0.0f;
			FVector MoveDir = CamRot.RotateVector(FVector::RightVector);
			Char->GetCharacterMovement()->AddInputVector(MoveDir * -1.0f);
		}
		else if (difference >= -180.0f - HeavyAngleTolerance && difference <= -180.0f + HeavyAngleTolerance)
		{
			FRotator CamRot = CharacterController->PlayerCameraManager->GetCameraRotation();
			CamRot.Pitch = 0.0f;
			FVector MoveDir = CamRot.RotateVector(FVector::RightVector);
			Char->GetCharacterMovement()->AddInputVector(MoveDir * 1.0f);
		}
		else
		{
			float coeff = GetRotateMultiplayer(difference) * RotationSpeed * GetWorld()->GetDeltaSeconds();
			//MAYBE USE DELTA TIME AND OBJECT WEIGHT
			FRotator	rotation = FQuat(FVector::UpVector, FMath::DegreesToRadians(coeff)).Rotator();
			FVector		holdingToCharac = Char->GetActorLocation() - holdingObjectLocation;
			FVector		holdingToNewLoc = rotation.RotateVector(holdingToCharac);
			FVector		finalLocation = holdingToNewLoc + holdingObjectLocation;
			FRotator	finalRotation = rotation + Char->GetActorRotation();
			Char->SetActorLocationAndRotation(finalLocation, finalRotation.Quaternion(), true, nullptr, ETeleportType::TeleportPhysics);
		}
	}
}

float	UMoveComponent::GetCameraTargetDiffAngle() const
{
	ACharacter* Char = Cast<ACharacter>(GetOwner());
	APlayerController* CharacterController = Cast<APlayerController>(Char->GetController());

	FRotator CamRot = CharacterController->PlayerCameraManager->GetCameraRotation();
	return CamRot.Yaw - Char->GetActorRotation().Yaw + 90.0f;
}

float	UMoveComponent::GetRotateMultiplayer(float value) const
{
	if (value < -180.0f)
		value += 360.0f;
	return value > 0.0f ? 1.0f : -1.0f;
}

void	UMoveComponent::EnableMovingHeavyObjectMode()
{
	isMovingHeavyObject = true;
	ACharacter* Char = Cast<ACharacter>(GetOwner());
	if (Char)
		Char->GetCharacterMovement()->bOrientRotationToMovement = false;
}

void	UMoveComponent::DisableMovingHeavyObjectMode()
{
	isMovingHeavyObject = false;
	ACharacter* Char = Cast<ACharacter>(GetOwner());
	if (Char)
		Char->GetCharacterMovement()->bOrientRotationToMovement = true;
}