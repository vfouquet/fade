// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BreathGameModeBase.generated.h"

class UStoryChapter;
class UDataTable;

/**
 * 
 */
UCLASS()
class BREATH_API ABreathGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABreathGameModeBase(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool	bGameStarted;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStoryChapter*	CurrentChapter;

	UFUNCTION(BlueprintCallable)
	void	SaveGame();
	//UFUNCTION(exec)
	void	LoadGame();
	//UFUNCTION(exec)
	void	LoadGameToChapter(UStoryChapter* Chapter);

protected:
	

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TAssetPtr<UWorld>	PersistentLevel;

};
