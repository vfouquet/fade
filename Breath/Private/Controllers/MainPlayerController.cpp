// Fill out your copyright notice in the Description page of Project Settings.

#include "MainPlayerController.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/SpringArmComponent.h"

#include "Cameras/CameraRailManager.h"

#include "MainCharacter.h"

#include "Camera/CameraActor.h"

void AMainPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (CameraActorClass != nullptr)
	{
		CameraActor = GetWorld()->SpawnActor<ACameraActor>(CameraActorClass, this->GetActorTransform());
	}
	else
	{
		CameraActor = GetWorld()->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), this->GetActorTransform());
	}

#if WITH_EDITOR
	CameraActor->SetActorLabel(TEXT("RailCamera"));
#endif
	this->SetViewTarget(CameraActor);
}

ACameraActor* AMainPlayerController::GetCameraActor()
{
	return CameraActor;
}

void AMainPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (InputComponent != nullptr)
	{
		InputComponent->BindAxis("MoveForward", this, &AMainPlayerController::MoveForward);
		InputComponent->BindAxis("MoveRight", this, &AMainPlayerController::MoveRight);
		//InputComponent->BindAxis("LookHorizontal", this, &AMainPlayerController::RotateHorizontal);
		//InputComponent->BindAxis("LookVertical", this, &AMainPlayerController::RotateVertical);

		InputComponent->BindAction("Jump", IE_Pressed, this, &AMainPlayerController::Jump);
		InputComponent->BindAction("Action", IE_Pressed, this, &AMainPlayerController::Action);
		InputComponent->BindAction("Grab", IE_Pressed, this, &AMainPlayerController::BeginGrab);
		InputComponent->BindAction("Grab", IE_Released, this, &AMainPlayerController::StopGrab);
#if WITH_EDITOR
		InputComponent->BindAction("DebugPauseEditor", IE_Pressed, this, &AMainPlayerController::DebugPauseEditor);
#endif
	}
}

void AMainPlayerController::Possess(APawn* aPawn)
{
	Super::Possess(aPawn);

	MainCharacter = Cast<AMainCharacter>(aPawn);

	if (MainCharacter != nullptr)
		SpringArmComponent = MainCharacter->FindComponentByClass<USpringArmComponent>();
}

void AMainPlayerController::UnPossess()
{
	Super::UnPossess();

	SpringArmComponent = nullptr;
	MainCharacter = nullptr;
}


void	AMainPlayerController::MoveForward(float Value)
{
	if (MainCharacter != nullptr)
	{
		updateCharacterValues();

		if (Value < WalkStickThreshold && Value > -WalkStickThreshold)
		{
			MainCharacter->Move(FVector::ZeroVector);
			return;
		}

		FRotator CamRot = GetCameraRotation();
		CamRot.Pitch = 0.0f;
		FVector MoveDir = CamRot.Vector();
		if (Value < 0.0f)
			MainCharacter->Move(MoveDir * -1.0f);
		else
			MainCharacter->Move(MoveDir);
	}
}

void	AMainPlayerController::MoveRight(float Value)
{
	if (MainCharacter != nullptr)
	{
		updateCharacterValues();
		if (Value < WalkStickThreshold && Value > -WalkStickThreshold)
		{
			MainCharacter->Move(FVector::ZeroVector);
			return;
		}

		FRotator CamRot = GetCameraRotation();
		CamRot.Pitch = 0.0f;
		FVector MoveDir = CamRot.RotateVector(FVector::RightVector);
		if (Value < 0.0f)
			MainCharacter->Move(MoveDir * -1.0f);
		else
			MainCharacter->Move(MoveDir);
	}
}

void	AMainPlayerController::RotateHorizontal(float Value)
{
	if (MainCharacter != nullptr)
		MainCharacter->RotateHorizontal(Value);
}

void	AMainPlayerController::RotateVertical(float Value)
{
	if (MainCharacter != nullptr)
		MainCharacter->RotateVertical(Value);
}

void AMainPlayerController::Jump()
{
	if (MainCharacter != nullptr)
	{
		if (MainCharacter->CanThrow())
			MainCharacter->Throw();
		else
			MainCharacter->Jump();
	}
}

void	AMainPlayerController::Action()
{
	if (MainCharacter)
		MainCharacter->Action();
}

void	AMainPlayerController::BeginGrab()
{
	if (MainCharacter != nullptr)
		MainCharacter->BeginGrab();
}

void	AMainPlayerController::StopGrab()
{
	if (MainCharacter != nullptr)
		MainCharacter->StopGrab();
}

#if WITH_EDITOR
#include "UnrealEd.h"
void	AMainPlayerController::DebugPauseEditor()
{
	GUnrealEd->PlayWorld->bDebugPauseExecution = true;
}
#endif
	
void	AMainPlayerController::updateCharacterValues()
{
	float stickLength = getStickLength();
	if (stickLength <= JogStickThreshold)
		MainCharacter->SetWalkMode();
	else
		MainCharacter->SetJogMode();

	if (FMath::IsNearlyZero(stickLength))
	{
		MainCharacter->SetRotatingLeft(false);
		MainCharacter->SetRotatingRight(false);
		MainCharacter->SetPushingAxis(0.0f);
		return;
	}

	float	inputAngle = FMath::RadiansToDegrees(FMath::Atan2(GetInputAxisValue("MoveForward"), GetInputAxisValue("MoveRight")));
	float cameraDiffAngle = GetCameraRotation().Yaw - MainCharacter->GetActorRotation().Yaw;

	inputAngle += 90.0f;
	if (inputAngle < 0.0f)
		inputAngle += 360.0f;

	if (cameraDiffAngle < 0.0f)
		cameraDiffAngle += 360.0f;

	float difference = inputAngle - cameraDiffAngle;

	if (difference < 0.0f)
		difference += 360.0f;

	if (difference >= 180.0f - HeavyAngleTolerance && difference <= 180.0f + HeavyAngleTolerance)
	{
		MainCharacter->SetRotatingLeft(false);
		MainCharacter->SetRotatingRight(false);
		MainCharacter->SetPushingAxis(1.0f);
	}
	else if (difference >= 360.0f - HeavyAngleTolerance || difference <= HeavyAngleTolerance)
	{
		MainCharacter->SetRotatingLeft(false);
		MainCharacter->SetRotatingRight(false);
		MainCharacter->SetPushingAxis(-1.0f);
	}
	else
	{
		MainCharacter->SetRotatingRight(difference > HeavyAngleTolerance && difference < 180.0f - HeavyAngleTolerance);
		MainCharacter->SetRotatingLeft(difference > 180.0f + HeavyAngleTolerance && difference < 360.0f - HeavyAngleTolerance);
		MainCharacter->SetPushingAxis(0.0f);
	}
}