// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "MainPlayerStart.generated.h"

class ACameraRailManager;

/**
 * 
 */
UCLASS()
class BREATH_API AMainPlayerStart : public APlayerStart
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TAssetPtr<ACameraRailManager>	StartCameraRailManager;
	
};
