// Fill out your copyright notice in the Description page of Project Settings.

#include "CameraSettingsZone.h"

#include "Cameras/CameraRailManager.h"

// Sets default values
ACameraSettingsZone::ACameraSettingsZone()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ACameraSettingsZone::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACameraSettingsZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACameraSettingsZone::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (this->bInCameraSettings == true && CameraRailManager.IsValid())
	{
		if (OtherActor == CameraRailManager->PlayerActor && CameraRailManager->RailCamera != nullptr)
		{
			CameraRailManager->RailCamera->CameraSettings = this->InCameraSettings;
		}
	}
}

void ACameraSettingsZone::NotifyActorEndOverlap(AActor* OtherActor)
{
	Super::NotifyActorEndOverlap(OtherActor);

	if (this->bOutCameraSettings == true && CameraRailManager.IsValid())
	{
		if (OtherActor == CameraRailManager->PlayerActor && CameraRailManager->RailCamera != nullptr)
		{
			CameraRailManager->RailCamera->CameraSettings = this->OutCameraSettings;
		}
	}
}