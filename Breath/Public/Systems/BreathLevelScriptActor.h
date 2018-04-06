// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/LevelScriptActor.h"
#include "BreathLevelScriptActor.generated.h"

class ACameraRailManager;

/**
 * 
 */
UCLASS()
class BREATH_API ABreathLevelScriptActor : public ALevelScriptActor
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ACameraRailManager*	CameraRailManager;
	
protected:
	virtual void BeginPlay() override;

};
