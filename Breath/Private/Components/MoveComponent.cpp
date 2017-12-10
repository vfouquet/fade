// Fill out your copyright notice in the Description page of Project Settings.

#include "MoveComponent.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "DrawDebugHelpers.h"

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

	if (isMovingHeavyObject)
	{
		if (currentInputValue == FVector2D::ZeroVector)
			return;

		ACharacter* Char = Cast<ACharacter>(GetOwner());
		
		float	inputAngle = GetInputAngle();
		APlayerController* CharacterController = Cast<APlayerController>(Char->GetController());

		FRotator CamRot = CharacterController->PlayerCameraManager->GetCameraRotation();
		FRotator playerForward = (holdingObjectLocation - Char->GetActorLocation()).Rotation();
		float cameraDiffAngle = CamRot.Yaw - Char->GetActorRotation().Yaw - 90.0f;
		//float cameraDiffAngle = CamRot.Yaw - playerForward.Yaw;
		
		float difference = inputAngle - cameraDiffAngle;
		if (difference < -180.0f)
			difference += 360.0f;
		UE_LOG(LogTemp, Warning, TEXT("Diff : %f"), difference);

		if (difference >= -HeavyAngleTolerance && difference <= HeavyAngleTolerance)
		{
			FVector MoveDir = Char->GetActorRotation().Vector();
			Char->GetCharacterMovement()->AddInputVector(MoveDir * -1.0f);
		}
		else if (difference >= -180.0f - HeavyAngleTolerance && difference <= -180.0f + HeavyAngleTolerance)
		{
			FVector MoveDir = Char->GetActorRotation().Vector();
			Char->GetCharacterMovement()->AddInputVector(MoveDir);
		}
		else
		{
			float angle = RotationSpeed * GetWorld()->GetDeltaSeconds();	
			angle *= (difference > 0.0f) ? -1.0f : 1.0f;
			//MAYBE USE OBJECT WEIGHT
			FRotator	rotation = FQuat(FVector::UpVector, FMath::DegreesToRadians(angle)).Rotator();
			FVector		holdingToCharac = Char->GetActorLocation() - holdingObjectLocation;
			FVector		holdingToNewLoc = rotation.RotateVector(holdingToCharac);
			FVector		finalLocation = holdingToNewLoc + holdingObjectLocation;
			FRotator	finalRotation = rotation + Char->GetActorRotation();
			Char->SetActorLocationAndRotation(finalLocation, finalRotation.Quaternion(), true, nullptr, ETeleportType::None);
		}

		CamRot.Pitch = 0.0f;
		CamRot.Roll = 0.0f;
		FRotator debugRotator;
		debugRotator.Yaw = difference;
		DrawDebugLine(GetWorld(), Char->GetActorLocation(), Char->GetActorLocation() + debugRotator.RotateVector(CamRot.Vector()) * 100.0f, FColor::Blue);
		DrawDebugLine(GetWorld(), Char->GetActorLocation(), Char->GetActorLocation() + debugRotator.RotateVector(CamRot.Vector()) * -100.0f, FColor::Magenta);
	}
}

void UMoveComponent::MoveForward(float Value)
{
	if (isBlocked)
		return;

	ACharacter* Char = Cast<ACharacter>(GetOwner());
	APlayerController* CharacterController = Cast<APlayerController>(Char->GetController());

	if (isMovingHeavyObject)
		currentInputValue.Y = Value;
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
		float realValue = (Value <= JogThreshold) ? JogThreshold : 1.0f;
		realValue *= (neg) ? -1.0f : 1.0f;

		FRotator CamRot = CharacterController->PlayerCameraManager->GetCameraRotation();
		CamRot.Pitch = 0.0f;
		FVector MoveDir = CamRot.Vector();
		Char->GetCharacterMovement()->AddInputVector(MoveDir * realValue);
	}
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
		currentInputValue.X = Value;
}

float	UMoveComponent::GetCameraTargetDiffAngle() const
{
	ACharacter* Char = Cast<ACharacter>(GetOwner());
	APlayerController* CharacterController = Cast<APlayerController>(Char->GetController());

	FRotator CamRot = CharacterController->PlayerCameraManager->GetCameraRotation();
	return CamRot.Yaw - Char->GetActorRotation().Yaw;
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