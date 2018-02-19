// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Cameras/RailCamera.h"
#include "StoryChapter.generated.h"

class AMainPlayerStart;

/**
 * 
 */
UCLASS()
class BREATH_API UStoryChapter : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info")
	FString	Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info")
	TAssetPtr<UTexture> Screenshot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info")
	bool	bAvailableToSelection;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Game")
	TAssetPtr<AMainPlayerStart>	Spawn;

};
