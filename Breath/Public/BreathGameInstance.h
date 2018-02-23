// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "BreathGameInstance.generated.h"

class UStoryChapter;

/**
 * 
 */
UCLASS()
class BREATH_API UBreathGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
	UFUNCTION(exec, BlueprintCallable)
	void	SaveGame();
	UFUNCTION(exec, BlueprintCallable)
	void	LoadGame();
	UFUNCTION(exec, BlueprintCallable)
	void	LoadGameToChapter(FString ChapterToLoad);
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UWorld*	PersistentLevel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TAssetPtr<UStoryChapter>	CurrentChapter;

public:
	virtual void LoadComplete(const float LoadTime, const FString& MapName) override;


	virtual void Init() override;

};
