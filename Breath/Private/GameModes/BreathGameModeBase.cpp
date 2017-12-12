// Fill out your copyright notice in the Description page of Project Settings.

#include "BreathGameModeBase.h"

#include "MainPlayerStart.h"
#include "Controllers/MainPlayerController.h"
#include "Cameras/CameraRailManager.h"
#include "Cameras/RailCamera.h"



void ABreathGameModeBase::RestartPlayerAtPlayerStart(AController* NewPlayer, AActor* StartSpot)
{
	Super::RestartPlayerAtPlayerStart(NewPlayer, StartSpot);

	AMainPlayerController*	PC = Cast<AMainPlayerController>(NewPlayer);
	AMainPlayerStart* MainPlayerStart = Cast<AMainPlayerStart>(StartSpot);

	if (PC != nullptr && PC->GetCameraActor() != nullptr)
	{
		if (MainPlayerStart != nullptr && MainPlayerStart->StartCameraRailManager.IsValid())
		{
			ARailCamera* RailCamera = Cast<ARailCamera>(PC->GetCameraActor());

			if (RailCamera != nullptr)
			{
				RailCamera->AttachToRailWithPlayer(MainPlayerStart->StartCameraRailManager.Get(), PC->GetPawn());
			}
		}
	}
}
