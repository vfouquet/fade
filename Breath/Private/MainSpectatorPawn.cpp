// Fill out your copyright notice in the Description page of Project Settings.

#include "MainSpectatorPawn.h"

#include "GameFramework/SpringArmComponent.h"
#include "Engine/World.h"

void	AMainSpectatorPawn::BeginPlay()
{
	Super::BeginPlay();

	auto* springArm = FindComponentByClass<USpringArmComponent>();
	if (springArm)
		SpringArmLength = springArm->TargetArmLength;
}

void	AMainSpectatorPawn::SetPhotoMode()
{
	auto* springArm = FindComponentByClass<USpringArmComponent>();
	if (springArm)
	{
		springArm->TargetArmLength = SpringArmLength;
		springArm->bUsePawnControlRotation = true;
	}
	bIsPhoto = true;
}

void	AMainSpectatorPawn::SetBasicMode()
{
	bIsPhoto = false;
	auto* springArm = FindComponentByClass<USpringArmComponent>();
	if (springArm)
	{
		springArm->TargetArmLength = 0.0f;
		springArm->bUsePawnControlRotation = false;
	}
}

void	AMainSpectatorPawn::MoveForward(float value)
{
	if (bIsPhoto)
		return;
	FVector	dir = FVector::ZeroVector;
	auto* springArm = FindComponentByClass<USpringArmComponent>();
	if (springArm)
		dir = springArm->GetForwardVector();
	AddMovementInput(dir * value * MoveSpeed * GetWorld()->GetDeltaSeconds());
}

void	AMainSpectatorPawn::MoveRight(float value)
{
	if (bIsPhoto)
		return;
	FVector	dir = FVector::ZeroVector;
	auto* springArm = FindComponentByClass<USpringArmComponent>();
	if (springArm)
		dir = springArm->GetRightVector();
	AddMovementInput(dir * value * MoveSpeed * GetWorld()->GetDeltaSeconds());
}

void	AMainSpectatorPawn::RotateHorizontal(float value)
{
	AddControllerYawInput(value * RotateSpeed * GetWorld()->GetDeltaSeconds());
}

void	AMainSpectatorPawn::RotateVertical(float value)
{
	AddControllerPitchInput(value * RotateSpeed * GetWorld()->GetDeltaSeconds());
}