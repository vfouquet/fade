// Fill out your copyright notice in the Description page of Project Settings.

#include "BreathGameModeBase.h"

#include "Kismet/GameplayStatics.h"

#include "MainPlayerStart.h"
#include "Controllers/MainPlayerController.h"
#include "Cameras/CameraRailManager.h"
#include "Cameras/RailCamera.h"
#include "BreathSaveGame.h"
#include "Story/StoryChapter.h"


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

void ABreathGameModeBase::SaveGame()
{
	UBreathSaveGame* SaveGameInstance = Cast<UBreathSaveGame>(UGameplayStatics::CreateSaveGameObject(UBreathSaveGame::StaticClass()));
	SaveGameInstance->Chapter = this->CurrentChapter;

	if (UGameplayStatics::SaveGameToSlot(SaveGameInstance, "MainSlot", 0))
	{
		UE_LOG(LogTemp, Warning, TEXT("Game saved."));
		//UE_LOG(LogTemp, Warning, TEXT("Current Chatper Name : %s"), this->CurrentChapter->Name);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to save game!"));
	}
	
}

void ABreathGameModeBase::LoadGame()
{
	UBreathSaveGame* LoadGameInstance = LoadGameInstance = Cast<UBreathSaveGame>(UGameplayStatics::LoadGameFromSlot("MainSlot", 0));;
	
	if (LoadGameInstance != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Game loaded."));

		if (LoadGameInstance->Chapter.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("Chapter name: %s"), *LoadGameInstance->Chapter->Name);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("No Chapter"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load game!"));
	}
}

void ABreathGameModeBase::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);
}

void ABreathGameModeBase::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
}
