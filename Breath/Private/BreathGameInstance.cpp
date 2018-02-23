// Fill out your copyright notice in the Description page of Project Settings.

#include "BreathGameInstance.h"

#include "Engine/World.h"
#include "Engine/StreamableManager.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/StringAssetReference.h"
#include "FileManager.h"
#include "Misc/Paths.h"

#include "GameModes/BreathGameModeBase.h"
#include "Systems/BreathSaveGame.h"
#include "Story/StoryChapter.h"

void UBreathGameInstance::SaveGame()
{
	
	ABreathGameModeBase* GM = Cast<ABreathGameModeBase>(UGameplayStatics::GetGameMode(this));

	if (GM != nullptr)
	{
		GM->SaveGame();
	}
}

void UBreathGameInstance::LoadGame()
{
	UGameplayStatics::OpenLevel(this, "DevSandbox");

	ABreathGameModeBase* GM = Cast<ABreathGameModeBase>(UGameplayStatics::GetGameMode(this));

	if (GM != nullptr)
	{
		GM->LoadGame();
	}
	else
	{
		// return to main menu
	}
}

void UBreathGameInstance::LoadGameToChapter(FString ChapterToLoad)
{
	UE_LOG(LogTemp, Warning, TEXT("Loading Chapter: %s"), *ChapterToLoad);
	FStreamableManager AssetLoader;
	FStringAssetReference MyAssetPath("/Game/Gameplay/Story/Chapters/" + ChapterToLoad);
	UObject* MyAsset = MyAssetPath.TryLoad();
	UStoryChapter*	Chapter = Cast<UStoryChapter>(MyAsset);

	if (Chapter != nullptr)
	{
		this->CurrentChapter = Chapter;

		TArray<FString> MapFiles;

		FString SearchedLevelName = Chapter->LevelName + TEXT(".umap");

		IFileManager::Get().FindFilesRecursive(MapFiles, *FPaths::GameContentDir(), *SearchedLevelName, true, false, false);

		if (MapFiles.Num() > 0)
		{
			UGameplayStatics::OpenLevel(this, *Chapter->LevelName);
		}
		else
		{
			// error map doesn't exists
		}
	}
	else
	{
		// error chapter is invalid
	}
}

void UBreathGameInstance::LoadComplete(const float LoadTime, const FString& MapName)
{
	Super::LoadComplete(LoadTime, MapName);

	ABreathGameModeBase* GM = Cast<ABreathGameModeBase>(UGameplayStatics::GetGameMode(this));

	if (GM != nullptr)
	{
		GM->LoadGameToChapter(CurrentChapter.Get());
	}
	else
	{
		// return to main menu
	}
}

void UBreathGameInstance::Init()
{
	Super::Init();

	TArray<FString> ChapterFiles;
	IFileManager::Get().FindFilesRecursive(ChapterFiles, *(FPaths::GameContentDir() + "/Gameplay/Story/Chapters/"), TEXT("*.uasset"), true, false, false);

	for (FString ChapterName : ChapterFiles)
	{
		FStreamableManager AssetLoader;
		FStringAssetReference MyAssetPath("/Game/Gameplay/Story/Chapters/" + ChapterName + ".uasset");
		UObject* MyAsset = MyAssetPath.TryLoad();
	}
}

