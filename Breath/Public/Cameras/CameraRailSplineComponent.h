// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "CameraRailSplineComponent.generated.h"

/**
 * 
 */
UCLASS()
class BREATH_API UCameraRailSplineComponent : public USplineComponent
{
	GENERATED_BODY()
	
	
public:
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override;

};
