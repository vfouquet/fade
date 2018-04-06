// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Cameras/RailCamera.h"
#include "BreathSaveGame.generated.h"

class UStoryChapter;

/**
 * 
 */
UCLASS()
class BREATH_API UBreathSaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:
	UPROPERTY(VisibleAnywhere)
	UStoryChapter*	Chapter;
	
	UBreathSaveGame();
};
