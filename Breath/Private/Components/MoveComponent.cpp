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

		ACharacter* Char = Cast<ACharacter>(GetOwner());
		APlayerController* CharacterController = Cast<APlayerController>(Char->GetController());

		FRotator CamRot = CharacterController->PlayerCameraManager->GetCameraRotation();
		CamRot.Pitch = 0.0f;
		FVector MoveDir = CamRot.Vector();
		Char->GetCharacterMovement()->AddInputVector(MoveDir * realValue);
	}
	else
	{
		currentInputValue.Y = Value;
		ACharacter* Char = Cast<ACharacter>(GetOwner());
		APlayerController* CharacterController = Cast<APlayerController>(Char->GetController());

		FRotator CamRot = CharacterController->PlayerCameraManager->GetCameraRotation();
		float diff = CamRot.Yaw - Char->GetActorRotation().Yaw + 90.0f;
		float	inputAngle = FMath::RadiansToDegrees(FMath::Atan2(currentInputValue.Y, currentInputValue.X));
		FVector2D	targetAngles(diff - HeavyAngleTolerance, diff + HeavyAngleTolerance);
		if (inputAngle >= targetAngles.X && inputAngle <= targetAngles.Y)
		{
			UE_LOG(LogTemp, Warning, TEXT("THIS IS OK"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Diff : %f   -   Input Angle : %f"), diff, inputAngle);
		}
	}
}

void UMoveComponent::MoveRight(float Value)
{
	if (isBlocked)
		return;

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
		ACharacter* Char = Cast<ACharacter>(GetOwner());
		APlayerController* CharacterController = Cast<APlayerController>(Char->GetController());

		FRotator CamRot = CharacterController->PlayerCameraManager->GetCameraRotation();
		CamRot.Pitch = 0.0f;
		FVector MoveDir = CamRot.RotateVector(FVector::RightVector);
		Char->GetCharacterMovement()->AddInputVector(MoveDir * realValue);
	}
	else
	{
		currentInputValue.X = Value;
		ACharacter* Char = Cast<ACharacter>(GetOwner());
		APlayerController* CharacterController = Cast<APlayerController>(Char->GetController());

		FRotator CamRot = CharacterController->PlayerCameraManager->GetCameraRotation();
		float diff = CamRot.Yaw - Char->GetActorRotation().Yaw + 90.0f;
		float	inputAngle = FMath::RadiansToDegrees(FMath::Atan2(currentInputValue.Y, currentInputValue.X));
		FVector2D	targetAngles(diff - HeavyAngleTolerance, diff + HeavyAngleTolerance);
		if (inputAngle >= targetAngles.X && inputAngle <= targetAngles.Y)
		{
			UE_LOG(LogTemp, Warning, TEXT("THIS IS OK"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Diff : %f   -   Input Angle : %f"), diff, inputAngle);
		}
	}
}