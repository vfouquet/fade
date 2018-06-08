// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "OptionsUtilities.generated.h"

/**
 * 
 */
UCLASS()
class BREATH_API UOptionsUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "CustomOptions")
	static void	SaveDynamicFoliageToConfig();
	UFUNCTION(BlueprintCallable, Category = "CustomOptions")
	static void	SaveCurrentAudioLanguageToConfig();
	UFUNCTION(BlueprintCallable, Category = "CustomOptions")
	static void SaveCurrentVolumeToConfig();
	UFUNCTION(BlueprintCallable, Category = "CustomOptions")
	static void	SetAudioLanguage(FString newVal);
	UFUNCTION(BlueprintCallable, Category = "CustomOptions")
	static void	SetDynamicFoliage(bool newVal);
	UFUNCTION(BlueprintCallable, Category = "CustomOptions")
	static void	SetVolumeValues(FVector values);
	UFUNCTION(BlueprintCallable, Category = "CustomOptions")
	static	FString	GetCurrentAudioLanguage();
	UFUNCTION(BlueprintCallable, Category = "CustomOptions")
	static	bool	GetCurrentDynamicFoliage();
	UFUNCTION(BlueprintCallable, Category = "CustomOptions")
	static	FVector	GetCurrentVolumeValues();

private:
	static	FString	tempCulture;
	static	bool	tempDynamicFoliage;
	static	FVector	tempVolumeValues;
};
