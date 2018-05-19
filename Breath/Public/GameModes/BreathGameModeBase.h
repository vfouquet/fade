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
	virtual void InitGameState() override;

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

	UFUNCTION(exec, BlueprintCallable)
	void	SetCameraTPS(bool value);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TPS")
	TSubclassOf<ACharacter>	TPSCharacter;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TPS")
	TSubclassOf<ACharacter>	ClassicCharacter;

protected:
	

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TAssetPtr<UWorld>	PersistentLevel;

};
