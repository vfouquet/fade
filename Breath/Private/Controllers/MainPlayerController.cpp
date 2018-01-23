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
		MainCharacter->MoveForward(Value);
}

void	AMainPlayerController::MoveRight(float Value)
{
	if (MainCharacter != nullptr)
		MainCharacter->MoveRight(Value);
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