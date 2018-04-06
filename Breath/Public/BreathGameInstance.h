// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
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

	UFUNCTION(BlueprintCallable)
	void	SetCurrentChapter(UStoryChapter* Chapter) { this->CurrentChapter = CurrentChapter; }

private:
	UFUNCTION(exec, BlueprintCallable)
	void	SaveGame();
	UFUNCTION(exec, BlueprintCallable)
	void	LoadGame();
	UFUNCTION(exec, BlueprintCallable)
	void	LoadGameToChapter(FString ChapterToLoad);
	
	void	LoadChapter(UStoryChapter* Chapter);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UWorld*	PersistentLevel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UStoryChapter*	CurrentChapter;

	

	virtual void OnStart() override;

public:
	virtual void LoadComplete(const float LoadTime, const FString& MapName) override;


	virtual void Init() override;

};
