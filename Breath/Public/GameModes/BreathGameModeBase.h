// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BreathGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class BREATH_API ABreathGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
	
	
	
public:
	virtual void RestartPlayerAtPlayerStart(AController* NewPlayer, AActor* StartSpot) override;

};
