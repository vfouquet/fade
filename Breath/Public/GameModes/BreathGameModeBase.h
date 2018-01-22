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
	virtual void RestartPlayerAtPlayerStart(AController* NewPlayer, AActor* StartSpot) override;

	UFUNCTION(exec)
	void	SaveGame();
	UFUNCTION(exec)
	void	LoadGame();

	virtual void RestartPlayer(AController* NewPlayer) override;



	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TAssetPtr<UStoryChapter>	CurrentChapter;
};
