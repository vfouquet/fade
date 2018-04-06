// Fill out your copyright notice in the Description page of Project Settings.

#include "BreathLevelScriptActor.h"

#include "Kismet/GameplayStatics.h"

#include "Cameras/CameraRailManager.h"
#include "Cameras/RailCamera.h"

void ABreathLevelScriptActor::BeginPlay()
{
	Super::BeginPlay();

	if (this->CameraRailManager != nullptr && this->CameraRailManager->RailCamera)
	{
		this->CameraRailManager->RailCamera->ChangePlayer(
			reinterpret_cast<AActor*>(UGameplayStatics::GetPlayerController(this, 0)->GetPawn()), true);
	}
}
