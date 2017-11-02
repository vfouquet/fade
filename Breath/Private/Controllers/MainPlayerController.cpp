// Fill out your copyright notice in the Description page of Project Settings.

#include "MainPlayerController.h"

#include "MainCharacter.h"
#include "MoveComponent.h"

void AMainPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (InputComponent != nullptr)
	{
		InputComponent->BindAxis("MoveForward", this, &AMainPlayerController::MoveForward);
		InputComponent->BindAxis("MoveRight", this, &AMainPlayerController::MoveRight);
		InputComponent->BindAxis("LookHorizontal", this, &AMainPlayerController::RotateHorizontal);
		InputComponent->BindAxis("LookVertical", this, &AMainPlayerController::RotateVertical);

		InputComponent->BindAction("Jump", IE_Pressed, this, &AMainPlayerController::Jump);
	}
}

void AMainPlayerController::Possess(APawn* aPawn)
{
	Super::Possess(aPawn);

	MainCharacter = Cast<AMainCharacter>(aPawn);

	if (MainCharacter != nullptr)
	{
		MoveComponent = MainCharacter->FindComponentByClass<UMoveComponent>();
	}
}

void AMainPlayerController::UnPossess()
{
	Super::UnPossess();

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
		MainCharacter->Jump();
	}
}

