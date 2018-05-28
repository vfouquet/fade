// Fill out your copyright notice in the Description page of Project Settings.

#include "CameraSettingsSwitch.h"

#include "Cameras/CameraRailManager.h"

// Sets default values
ACameraSettingsSwitch::ACameraSettingsSwitch()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bInCameraSettings = true;
	bOutCameraSettings = true;
}

// Called when the game starts or when spawned
void ACameraSettingsSwitch::BeginPlay()
{
	Super::BeginPlay();
	
	if (CameraRailManager.IsValid())
	{
		SplineInputKey = CameraRailManager->GetInputKeyAtWorldLocation(this->GetActorLocation());
	}
}

// Called every frame
void ACameraSettingsSwitch::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACameraSettingsSwitch::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

if (CameraRailManager.IsValid())
	{
		if (OtherActor == CameraRailManager->PlayerActor && CameraRailManager->RailCamera != nullptr)
		{

			if (CameraRailManager->GetInputKeyAtWorldLocation(CameraRailManager->PlayerActor->GetActorLocation()) > SplineInputKey && bOutCameraSettings == true)
			{
				CameraRailManager->RailCamera->ChangeSettings(OutCameraSettings);
			}
			else if (bInCameraSettings == true)
			{
				CameraRailManager->RailCamera->ChangeSettings(InCameraSettings);
			}
		}
	}
}

