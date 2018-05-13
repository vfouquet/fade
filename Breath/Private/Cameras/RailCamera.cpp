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
}

USceneComponent* ARailCamera::GetCameraArm()
{
	return CameraArm;
}

void ARailCamera::BeginPlay()
{
	Super::BeginPlay();

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
		this->CurrentRailManager->ChangePlayer(PlayerActor, bTeleport);
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
