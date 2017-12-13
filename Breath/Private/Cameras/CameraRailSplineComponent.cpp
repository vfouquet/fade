// Fill out your copyright notice in the Description page of Project Settings.

#include "CameraRailSplineComponent.h"

#if WITH_EDITOR
void UCameraRailSplineComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	UE_LOG(LogTemp, Warning, TEXT("TestClick"));

}
#endif
