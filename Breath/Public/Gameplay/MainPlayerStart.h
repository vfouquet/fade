// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "Cameras/RailCamera.h"
#include "MainPlayerStart.generated.h"

class ACameraRailManager;

/**
 * 
 */
UCLASS()
class BREATH_API AMainPlayerStart : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FCameraSettings	CameraSettings;
};
