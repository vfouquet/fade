#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

UENUM(BlueprintType)
enum class EChemicalType : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Wood UMETA(DisplayName = "Wood"),
	Fiber UMETA(DisplayName = "Fiber"),
	Rock UMETA(DisplayName = "Rock"),
	Ceramic UMETA(DisplayName = "Ceramic"),
	Water UMETA(DisplayName = "Water"),
	Fire UMETA(DisplayName = "Fire"),
	Oil UMETA(DisplayName = "Oil"),
	Wind UMETA(DisplayName = "Wind")
};

UENUM(BlueprintType)
enum class EChemicalState : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Fire UMETA(DisplayName = "Fire"),
	Ashes UMETA(DisplayName = "Ashes"),
	Wet UMETA(DisplayName = "Wet"),
	Oiled UMETA(DisplayName = "Oiled"),
	Broken UMETA(DisplayName = "Broken"),
	Extinguished UMETA(DisplayName = "Extinguished"),
	Dispersed UMETA(DisplayName = "Dispersed"),
	NoIdentity UMETA(DisplayName = "NoIdentity")
};

UENUM(BlueprintType)
enum class EChemicalTransformation : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Burning UMETA(DisplayName = "Burning"),
	Drowning UMETA(DisplayName = "Drowning"),
	Oiling UMETA(DisplayName = "Oiling")
};