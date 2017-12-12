// Fill out your copyright notice in the Description page of Project Settings.

#include "RailCamera.h"

#include "DrawDebugHelpers.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "Cameras/CameraRailManager.h"
#include "Cameras/CameraRailPoint.h"

ARailCamera::ARailCamera(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	CameraArm = CreateDefaultSubobject<USceneComponent>(TEXT("CameraArm"));
	CameraArm->SetupAttachment(this->GetRootComponent());

	this->GetCameraComponent()->SetupAttachment(CameraArm);
}

USceneComponent* ARailCamera::GetCameraArm()
{
	return CameraArm;
}

void ARailCamera::BeginPlay()
{
	Super::BeginPlay();
}

void ARailCamera::NotifyActorBeginOverlap(AActor* OtherActor)
{
	ACameraRailPoint*	RailPoint = Cast<ACameraRailPoint>(OtherActor);

	if (CurrentRailManager != nullptr && RailPoint != nullptr)
	{
		if (CurrentRailManager->GetCurrentInputKey() > RailPoint->SplinePoint.InputKey 
			&& RailPoint->bOutCameraSettings == true)
		{
			this->CameraSettings = RailPoint->OutCameraSettings;
		}
		else if (RailPoint->bInCameraSettings == true)
		{
			this->CameraSettings = RailPoint->InCameraSettings;
		}
	}
}

void ARailCamera::AttachToRail(ACameraRailManager* CameraRailManager)
{
	if (CurrentRailManager != CameraRailManager)
	{
		if (CurrentRailManager != nullptr)
		{
			CurrentRailManager->DetachCamera();
		}

		CurrentRailManager = CameraRailManager;

		if (CurrentRailManager != nullptr)
		{
			CurrentRailManager->AttachCamera(this);
		}
	}
}

void ARailCamera::AttachToRailWithPlayer(ACameraRailManager* CameraRailManager, AActor* PlayerActor)
{
	if (CurrentRailManager != CameraRailManager)
	{
		if (CurrentRailManager != nullptr)
		{
			CurrentRailManager->DetachCamera();
		}

		CurrentRailManager = CameraRailManager;

		if (CurrentRailManager != nullptr)
		{
			CurrentRailManager->AttachCamera(this, PlayerActor);
		}
	}
}

void ARailCamera::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	FVector DistanceOffset(-this->CameraSettings.CameraDistanceOffset, 0.0f, 0.0f);
	FVector CameraRelativeLocation = GetCameraComponent()->RelativeLocation;

	FVector InterpVec = FMath::VInterpConstantTo(CameraRelativeLocation, DistanceOffset, DeltaSeconds, this->CameraSettings.CameraDistanceOffsetSpeed);

	GetCameraComponent()->SetRelativeLocation(InterpVec);

#if WITH_EDITOR
	DrawDebugCamera(GetWorld(), this->GetActorLocation(), this->GetActorRotation(), this->GetCameraComponent()->FieldOfView, 1.f, FColor::Red);
	DrawDebugCamera(GetWorld(), GetCameraComponent()->GetComponentLocation(), this->GetActorRotation(), this->GetCameraComponent()->FieldOfView, 1.f, FColor::Red);
#endif
}
