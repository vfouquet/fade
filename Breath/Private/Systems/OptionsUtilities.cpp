// Fill out your copyright notice in the Description page of Project Settings.

#include "OptionsUtilities.h"

#include "ConfigCacheIni.h"

FString	UOptionsUtilities::tempCulture = FString();
bool	UOptionsUtilities::tempDynamicFoliage = false;
FVector	UOptionsUtilities::tempVolumeValues = FVector::ZeroVector;

void	UOptionsUtilities::SaveDynamicFoliageToConfig()
{
	if (!GConfig)
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't save dynamic foliage because GConfig is nullptr"));
		return;
	}

	GConfig->SetBool(TEXT("VFX"), TEXT("DynamicFoliage"), tempDynamicFoliage, GGameIni);

	GConfig->Flush(false, GGameIni);
}

void	UOptionsUtilities::SaveCurrentAudioLanguageToConfig()
{
	if (!GConfig)
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't save audio language because GConfig is nullptr"));
		return;
	}

	GConfig->SetString(TEXT("Audio"), TEXT("AudioLanguages"), *tempCulture, GGameIni);

	GConfig->Flush(false, GGameIni);
}


void	UOptionsUtilities::SaveCurrentVolumeToConfig()
{
	if (!GConfig)
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't save audio volumes because GConfig is nullptr"));
		return;
	}

	GConfig->SetVector(TEXT("Audio"), TEXT("VolumeValues"), tempVolumeValues, GGameIni);
	GConfig->Flush(false, GGameIni);
}

void	UOptionsUtilities::SetAudioLanguage(FString newVal)
{
	tempCulture = newVal;
}

void	UOptionsUtilities::SetDynamicFoliage(bool newVal)
{
	tempDynamicFoliage = newVal;
}

void	UOptionsUtilities::SetVolumeValues(FVector newVal)
{
	tempVolumeValues = newVal;
}

FString	UOptionsUtilities::GetCurrentAudioLanguage()
{
	if (!GConfig)
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't get current audio language because GConfig is nullptr"));
		return FString();
	}
	FString	currentVal;
	return GConfig->GetString(TEXT("Audio"), TEXT("AudioLanguages"), currentVal, GGameIni)? currentVal : FString();
}

bool	UOptionsUtilities::GetCurrentDynamicFoliage()
{
	if (!GConfig)
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't get current dynamic folioage because GConfig is nullptr"));
		return false;
	}
	bool	currentVal;
	return GConfig->GetBool(TEXT("VFX"), TEXT("DynamicFoliage"), currentVal, GGameIni)? currentVal : true;
}

FVector	UOptionsUtilities::GetCurrentVolumeValues()
{
	if (!GConfig)
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't get current audio volumes because GConfig is nullptr"));
		return FVector::ZeroVector;
	}

	FVector	currentVal;
	return GConfig->GetVector(TEXT("Audio"), TEXT("VolumeValues"), currentVal, GGameIni) ? currentVal : FVector(-1.0f);
}