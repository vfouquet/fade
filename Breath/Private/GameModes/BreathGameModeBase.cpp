// Fill out your copyright notice in the Description page of Project Settings.

#include "BreathGameModeBase.h"

#include "Engine/World.h"
#include "Engine/StreamableManager.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/StringAssetReference.h"
#include "GameFramework/Character.h"

#include "MainPlayerStart.h"
#include "Controllers/MainPlayerController.h"
#include "Cameras/CameraRailManager.h"
#include "Cameras/RailCamera.h"
#include "Systems/BreathSaveGame.h"
#include "Story/StoryChapter.h"
#include "Gameplay/MainPlayerStart.h"

#include "BreathGameInstance.h"

ABreathGameModeBase::ABreathGameModeBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	this->bStartPlayersAsSpectators = true;
	if (ClassicCharacter)
		DefaultPawnClass = ClassicCharacter->StaticClass();
}
	
void ABreathGameModeBase::InitGameState()
{
	UBreathGameInstance* gameInst = Cast<UBreathGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (gameInst && TPSCharacter && gameInst->IsCameraTPS())
		DefaultPawnClass = TPSCharacter->StaticClass();

	Super::InitGameState();
}

void ABreathGameModeBase::SaveGame()
{
	UBreathSaveGame* SaveGameInstance = Cast<UBreathSaveGame>(UGameplayStatics::CreateSaveGameObject(UBreathSaveGame::StaticClass()));
	SaveGameInstance->Chapter = this->CurrentChapter;

	if (UGameplayStatics::SaveGameToSlot(SaveGameInstance, "MainSlot", 0))
	{
		UE_LOG(LogTemp, Warning, TEXT("Game saved."));
		//UE_LOG(LogTemp, Warning, TEXT("Current Chapter Name : %s"), this->CurrentChapter->Name);
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

		if (LoadGameInstance->Chapter != nullptr)
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

void ABreathGameModeBase::LoadGameToChapter(UStoryChapter* Chapter)
{
	if (Chapter != nullptr)
	{
		Chapter->Spawn.LoadSynchronous();
		if (Chapter->Spawn.IsValid())
		{
			AMainPlayerController*  PC = Cast<AMainPlayerController>(UGameplayStatics::GetPlayerController(this, 0));

			if (PC != nullptr)
			{
				APawn* PlayerPawn = PC->GetPawnOrSpectator();

				if (Chapter->Spawn != nullptr)
				{
					if (PlayerPawn == nullptr)
					{
						FTransform	tempTransform = Chapter->Spawn->GetTransform();
						tempTransform.SetScale3D(FVector::OneVector);
						PlayerPawn = this->GetWorld()->SpawnActor<APawn>(this->DefaultPawnClass, tempTransform);
						PC->Possess(PlayerPawn);
					}
					else
					{
						FTransform transform = Chapter->Spawn->GetTransform();
						transform.SetScale3D(PlayerPawn->GetActorScale());
						PlayerPawn->SetActorTransform(transform);
					}

					ARailCamera* Camera = Cast<ARailCamera>(PC->GetCameraActor());
					
					if (Camera != nullptr)
					{
						Camera->ChangePlayer(PlayerPawn, true);
					}

					UE_LOG(LogTemp, Warning, TEXT("Chapter loaded."));
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("Failed to find spawn!"));
				}
			}
		}
	}
}

void	ABreathGameModeBase::SetCameraTPS(bool value)
{
	UBreathGameInstance* gameInst = Cast<UBreathGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (gameInst)
	{
		gameInst->SetCameraTPSValue(value);
		this->DefaultPawnClass = gameInst->IsCameraTPS() ? TPSCharacter : ClassicCharacter;
	}
}