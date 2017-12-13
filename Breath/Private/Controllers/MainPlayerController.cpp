// Fill out your copyright notice in the Description page of Project Settings.

#include "MainPlayerController.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/SpringArmComponent.h"

#include "Cameras/CameraRailManager.h"

#include "MainCharacter.h"
#include "MoveComponent.h"

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

	CameraActor->SetActorLabel(TEXT("RailCamera"));

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
		InputComponent->BindAction("Throw", IE_Pressed, this, &AMainPlayerController::Throw);
		InputComponent->BindAction("Stick", IE_Pressed, this, &AMainPlayerController::Stick);
	}
}

void AMainPlayerController::Possess(APawn* aPawn)
{
	Super::Possess(aPawn);

	MainCharacter = Cast<AMainCharacter>(aPawn);

	if (MainCharacter != nullptr)
	{
		MoveComponent = MainCharacter->FindComponentByClass<UMoveComponent>();
		SpringArmComponent = MainCharacter->FindComponentByClass<USpringArmComponent>();
	}
}

void AMainPlayerController::UnPossess()
{
	Super::UnPossess();

	SpringArmComponent = nullptr;
	MoveComponent = nullptr;
	MainCharacter = nullptr;
}


void	AMainPlayerController::MoveForward(float Value)
{
	if (MoveComponent != nullptr)
	{
		MoveComponent->MoveForward(Value);
	}
}

void	AMainPlayerController::MoveRight(float Value)
{
	if (MoveComponent != nullptr)
	{
		MoveComponent->MoveRight(Value);
	}
}

void	AMainPlayerController::RotateHorizontal(float Value)
{
	if (MainCharacter != nullptr)
	{
		MainCharacter->RotateHorizontal(Value);
	}
}

void	AMainPlayerController::RotateVertical(float Value)
{
	if (MainCharacter != nullptr)
	{
		MainCharacter->RotateVertical(Value);
	}
}

void AMainPlayerController::Jump()
{
	if (MainCharacter != nullptr)
	{
		if (!MainCharacter->Climb())
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

void	AMainPlayerController::Throw()
{
	if (MainCharacter)
		MainCharacter->Throw();
}

void	AMainPlayerController::Stick()
{
	if (MainCharacter)
		MainCharacter->Stick();
}