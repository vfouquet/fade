// Fill out your copyright notice in the Description page of Project Settings.

#include "RailCamera.h"

#include "DrawDebugHelpers.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/World.h"

#include "Cameras/CameraRailManager.h"
#include "Cameras/CameraRailPoint.h"
#include "Systems/BreathLevelScriptActor.h"

ARailCamera::ARailCamera(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	CameraArm = CreateDefaultSubobject<USceneComponent>(TEXT("CameraArm"));
	CameraArm->SetupAttachment(this->GetRootComponent());

	this->GetCameraComponent()->SetupAttachment(CameraArm);
	this->GetCameraComponent()->PostProcessBlendWeight = 0.0f;

	this->SpeedInterpTime = 1.0f;
	this->RotationSpeedInterpTime = 1.0f;
	this->DistanceSpeedInterpTime = 1.0f;
	this->HeightSpeedInterpTime = 1.0f;
	this->RailOffsetInterpTime = 1.0f;
}

USceneComponent* ARailCamera::GetCameraArm()
{
	return CameraArm;
}

void ARailCamera::BeginPlay()
{
	Super::BeginPlay();

	this->PreviousCameraSettings = this->CameraSettings;
	this->CurrentCameraSettings = this->CameraSettings;

	ABreathLevelScriptActor* LevelScript = Cast<ABreathLevelScriptActor>(this->GetWorld()->GetLevelScriptActor());
	if (LevelScript != nullptr && LevelScript->CameraRailManager != nullptr)
	{
		this->AttachToRail(LevelScript->CameraRailManager);
	}

#if WITH_EDITOR
	this->SetActorLabel(TEXT("RailCamera"));
#endif
}

void ARailCamera::NotifyActorBeginOverlap(AActor* OtherActor)
{
	ACameraRailPoint*	RailPoint = Cast<ACameraRailPoint>(OtherActor);

	if (CurrentRailManager != nullptr && RailPoint != nullptr)
	{
		if (CurrentRailManager->GetInputKeyAtWorldLocation(this->CameraArm->GetComponentToWorld().GetLocation()) > RailPoint->SplinePoint.InputKey 
			&& RailPoint->bOutCameraSettings == true)
		{
			this->ChangeSettings(RailPoint->OutCameraSettings);
		}
		else if (RailPoint->bInCameraSettings == true)
		{
			this->ChangeSettings(RailPoint->InCameraSettings);
		}
	}
}

void ARailCamera::AttachToRail(ACameraRailManager* CameraRailManager)
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

void ARailCamera::AttachToRailWithPlayer(ACameraRailManager* CameraRailManager, AActor* PlayerActor, bool bTeleport)
{
	if (CurrentRailManager != nullptr)
	{
		CurrentRailManager->DetachCamera();
	}

	CurrentRailManager = CameraRailManager;

	if (CurrentRailManager != nullptr)
	{
		CurrentRailManager->AttachCamera(this, PlayerActor, bTeleport);
	}
}

void	ARailCamera::ChangePlayer(AActor* PlayerActor, bool bTeleport)
{
	if (this->CurrentRailManager != nullptr && PlayerActor != nullptr)
	{
		GetCameraComponent()->SetRelativeLocation(FVector(-this->CameraSettings.CameraDistanceOffset, 0.0f, 0.0f));
		this->CurrentRailManager->ChangePlayer(PlayerActor, bTeleport);
	}
}

void ARailCamera::ChangeSettings(const FCameraSettings & Settings)
{
	this->SpeedInterpAlpha = 0.0f;
	this->RotationSpeedInterpAlpha = 0.0f;
	this->DistanceSpeedInterpAlpha = 0.0f;
	this->HeightSpeedInterpAlpha = 0.0f;
	this->RailOffsetInterpAlpha = 0.0f;

	this->PreviousCameraSettings = this->CurrentCameraSettings;
	this->CameraSettings = Settings;
}

void ARailCamera::UpdateCamera(float DeltaSeconds)
{
	SpeedInterpAlpha += (DeltaSeconds / SpeedInterpTime);
	RotationSpeedInterpAlpha += (DeltaSeconds / RotationSpeedInterpTime);
	DistanceSpeedInterpAlpha += (DeltaSeconds / DistanceSpeedInterpTime);
	HeightSpeedInterpAlpha += (DeltaSeconds / HeightSpeedInterpTime);
	RailOffsetInterpAlpha += (DeltaSeconds / RailOffsetInterpTime);

	if (SpeedInterpAlpha > 1.0f)
		SpeedInterpAlpha = 1.0f;
	if (RotationSpeedInterpAlpha > 1.0f)
		RotationSpeedInterpAlpha = 1.0f;
	if (DistanceSpeedInterpAlpha > 1.0f)
		DistanceSpeedInterpAlpha = 1.0f;
	if (HeightSpeedInterpAlpha > 1.0f)
		HeightSpeedInterpAlpha = 1.0f;
	if (RailOffsetInterpAlpha > 1.0f)
		RailOffsetInterpAlpha = 1.0f;


	this->CurrentCameraSettings.CameraSpeed = FMath::Lerp(this->PreviousCameraSettings.CameraSpeed, this->CameraSettings.CameraSpeed, this->SpeedInterpAlpha);
	this->CurrentCameraSettings.CameraRotationSpeed = FMath::Lerp(this->PreviousCameraSettings.CameraRotationSpeed, this->CameraSettings.CameraRotationSpeed, this->RotationSpeedInterpAlpha);
	this->CurrentCameraSettings.CameraDistanceOffsetSpeed = FMath::Lerp(this->PreviousCameraSettings.CameraDistanceOffsetSpeed, this->CameraSettings.CameraDistanceOffsetSpeed, this->DistanceSpeedInterpAlpha);
	this->CurrentCameraSettings.CameraHeightOffsetSpeed = FMath::Lerp(this->PreviousCameraSettings.CameraHeightOffsetSpeed, this->CameraSettings.CameraHeightOffsetSpeed, this->HeightSpeedInterpAlpha);
	this->CurrentCameraSettings.CameraRailOffset= FMath::Lerp(this->PreviousCameraSettings.CameraRailOffset, this->CameraSettings.CameraRailOffset, this->RailOffsetInterpAlpha);

	FVector DistanceOffset(-this->CameraSettings.CameraDistanceOffset, 0.0f, 0.0f);
	FVector CameraRelativeLocation = GetCameraComponent()->RelativeLocation;

	float x, z;
	x = CameraRelativeLocation.X;
	z = CameraRelativeLocation.Z;

	x = FMath::FInterpConstantTo(x, -this->CameraSettings.CameraDistanceOffset, DeltaSeconds, this->CurrentCameraSettings.CameraDistanceOffsetSpeed);
	z = FMath::FInterpConstantTo(z, this->CameraSettings.CameraHeightOffset, DeltaSeconds, this->CurrentCameraSettings.CameraHeightOffsetSpeed);

	GetCameraComponent()->SetRelativeLocation(FVector(x, 0.0f, z));

#if WITH_EDITOR
	DrawDebugCamera(GetWorld(), this->GetActorLocation(), this->GetActorRotation(), this->GetCameraComponent()->FieldOfView, 1.f, FColor::Red);
	DrawDebugCamera(GetWorld(), GetCameraComponent()->GetComponentLocation(), this->GetActorRotation(), this->GetCameraComponent()->FieldOfView, 1.f, FColor::Red);
#endif
}
