// Fill out your copyright notice in the Description page of Project Settings.

#include "BreathGameInstance.h"

#include "Engine/World.h"
#include "Engine/StreamableManager.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/StringAssetReference.h"
#include "FileManager.h"
#include "Misc/Paths.h"
#include "Engine/DataTable.h"
#include "Framework/Application/SlateApplication.h"
#include "../MoviePlayer/Public/MoviePlayer.h"


#include "GameModes/BreathGameModeBase.h"
#include "Systems/BreathSaveGame.h"
#include "Story/StoryChapter.h"
#include "Story/StoryManager.h"
#include "LoadingScreen.h"


void UBreathGameInstance::Init()
{
	Super::Init();

	for (int pos = 0; pos < 6; pos++)
		levelsReady[pos] = false;

	UE_LOG(LogTemp, Warning, TEXT("INIT"));

	if (this->DataTable != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("INIT OK"));
		FString ContextString;
		TArray<FStoryOrderData*> OutRowArray;

		this->DataTable->GetAllRows<FStoryOrderData>(ContextString, OutRowArray);


		for (FStoryOrderData* Row : OutRowArray)
		{
			UE_LOG(LogTemp, Warning, TEXT("TEST ROW"));
			if (Row != nullptr)
			{
				UE_LOG(LogTemp, Warning, TEXT("ROW OK"));
				//Row->Chapter.LoadSynchronous();
			}
			else
			{ 
				UE_LOG(LogTemp, Warning, TEXT("ROW NOK"));
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("INIT NULL"));
	}

	const ULoadingScreenSettings* Settings = GetDefault<ULoadingScreenSettings>();

	for (const FStringAssetReference& Ref : Settings->StartupScreen.Images)
	{
		Ref.TryLoad();
	}
	for (const FStringAssetReference& Ref : Settings->DefaultScreen.Images)
	{
		Ref.TryLoad();
	}

	if (IsMoviePlayerEnabled())
	{
		GetMoviePlayer()->OnPrepareLoadingScreen().AddUObject(this, &UBreathGameInstance::HandlePrepareLoadingScreen);
	}

	BeginLoadingScreen(Settings->StartupScreen);
}

void UBreathGameInstance::LoadComplete(const float LoadTime, const FString& MapName)
{
	Super::LoadComplete(LoadTime, MapName);

	ABreathGameModeBase* GM = Cast<ABreathGameModeBase>(UGameplayStatics::GetGameMode(this));

	if (GM != nullptr)
	{
		GM->LoadGameToChapter(CurrentChapter);
	}
	else
	{
		// return to main menu
	}
}

void	UBreathGameInstance::ResetLevelReadyTab()
{
	levelsReady[0] = false;
	levelsReady[1] = false;
	levelsReady[2] = false;
	levelsReady[3] = false;
	levelsReady[4] = false;
	levelsReady[5] = false;
}

bool	UBreathGameInstance::ToggleLevelReady(int value)
{
	if (value > 5 || value < 0)
		return false;
	levelsReady[value] = true;
	
	return LevelAllReady();
}

bool	UBreathGameInstance::LevelAllReady() const
{
	for (int pos = 0; pos < 6; pos++)
	{
		if (!levelsReady[pos])
			return false;
	}
	return true;
}

void UBreathGameInstance::HandlePrepareLoadingScreen()
{
	const ULoadingScreenSettings* Settings = GetDefault<ULoadingScreenSettings>();
	BeginLoadingScreen(Settings->DefaultScreen);
}

void UBreathGameInstance::BeginLoadingScreen(const FLoadingScreenDescription& ScreenDescription)
{
	FLoadingScreenAttributes LoadingScreen;
	LoadingScreen.MinimumLoadingScreenDisplayTime = ScreenDescription.MinimumLoadingScreenDisplayTime;
	LoadingScreen.bAutoCompleteWhenLoadingCompletes = ScreenDescription.bAutoCompleteWhenLoadingCompletes;
	LoadingScreen.bMoviesAreSkippable = ScreenDescription.bMoviesAreSkippable;
	LoadingScreen.bWaitForManualStop = ScreenDescription.bWaitForManualStop;
	LoadingScreen.MoviePaths = ScreenDescription.MoviePaths;
	LoadingScreen.PlaybackType = ScreenDescription.PlaybackType;

	if (ScreenDescription.bShowUIOverlay)
	{
		LoadingScreen.WidgetLoadingScreen = SNew(SSimpleLoadingScreen, ScreenDescription);
	}

	GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
}

bool UBreathGameInstance::IsChapterIsAlreadyPassed(UStoryChapter* Chapter)
{
	FString ContextString;
	TArray<FStoryOrderData*> OutRowArray;
	this->DataTable->GetAllRows<FStoryOrderData>(ContextString, OutRowArray);

	for (FStoryOrderData* Row : OutRowArray)
	{
		if (Row != nullptr)
		{
			if (Row->Chapter == this->CurrentChapter)
				return false;
			else if (Row->Chapter == Chapter)
				return true;
		}
	}

	return false;
}

void UBreathGameInstance::OnStart()
{
	Super::OnStart();

	UBreathSaveGame* LoadGameInstance = LoadGameInstance = Cast<UBreathSaveGame>(UGameplayStatics::LoadGameFromSlot("MainSlot", 0));

	if (LoadGameInstance != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Game loaded."));

		if (LoadGameInstance->Chapter != nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("Chapter name: %s"), *LoadGameInstance->Chapter->Name);
			this->CurrentChapter = LoadGameInstance->Chapter;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("No Chapter"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load save slot!"));
	}
}

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
	UBreathSaveGame* LoadGameInstance = LoadGameInstance = Cast<UBreathSaveGame>(UGameplayStatics::LoadGameFromSlot("MainSlot", 0));

	if (LoadGameInstance != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Game loaded."));

		if (LoadGameInstance->Chapter != nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("Chapter name: %s"), *LoadGameInstance->Chapter->Name);
			LoadChapter(LoadGameInstance->Chapter);
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

void UBreathGameInstance::LoadChapter(UStoryChapter* Chapter)
{
	if (Chapter != nullptr)
	{
		this->CurrentChapter = Chapter;
		UGameplayStatics::OpenLevel(this, *Chapter->LevelName);
	}
}