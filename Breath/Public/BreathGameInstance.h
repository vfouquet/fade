// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Engine/GameInstance.h"
#include "Widget.h"

#include "LoadingScreenSettings.h"

#include "BreathGameInstance.generated.h"

class UStoryChapter;
class UDataTable;

/**
 * 
 */
UCLASS()
class BREATH_API UBreathGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDataTable* DataTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UWidget>	LoadingScreen;

public:
	virtual void Init() override;

	UFUNCTION(BlueprintCallable)
	void	SetCurrentChapter(UStoryChapter* Chapter) { this->CurrentChapter = CurrentChapter; }

	virtual void LoadComplete(const float LoadTime, const FString& MapName) override;

	UFUNCTION()
	void HandlePrepareLoadingScreen();

	UFUNCTION()
	virtual void BeginLoadingScreen(const FLoadingScreenDescription& ScreenDescription);
	
	UFUNCTION(BlueprintPure)
	bool	IsCameraTPS() const { return bIsTPSCamera; }
	UFUNCTION(BlueprintCallable)
	void	SetCameraTPSValue(bool value) { bIsTPSCamera = value; }

	UFUNCTION(BlueprintCallable)
	bool	IsChapterIsAlreadyPassed(UStoryChapter* Chapter);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UWorld*	PersistentLevel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UStoryChapter*	CurrentChapter;

	bool	bIsTPSCamera = false;
protected:
	virtual void OnStart() override;

private:
	UFUNCTION(exec, BlueprintCallable)
	void	SaveGame();
	UFUNCTION(exec, BlueprintCallable)
	void	LoadGame();
	UFUNCTION(exec, BlueprintCallable)
	void	LoadGameToChapter(FString ChapterToLoad);

	void	LoadChapter(UStoryChapter* Chapter);

	UWidget*	CurrentScreenWidget;

};
