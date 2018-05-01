#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

UENUM(BlueprintType)
enum class EChemicalType : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Wood UMETA(DisplayName = "Wood"),
	Rock UMETA(DisplayName = "Rock"),
	Ceramic UMETA(DisplayName = "Ceramic"),
	Water UMETA(DisplayName = "Water"),
	Fire UMETA(DisplayName = "Fire"),
	Oil UMETA(DisplayName = "Oil"),
};

UENUM(BlueprintType)
enum class EChemicalState : uint8
{
	None = 0 UMETA(DisplayName = "None"), //ECHEMICALFIRE CASE -> NONE = BURNING
	Lit UMETA(DisplayName = "Lit"),
	Burning UMETA(DisplayName = "Burning"),
	Scorched UMETA(DisplayName = "Scorched"),
	Drenched UMETA(DisplayName = "Drenched"),
	Stained UMETA(DisplayName = "Stained"),
	NoIdentity UMETA(DisplayName = "NoIdentity"),
	Extinguished UMETA(DisplayName = "Extinguished"), //ECHEMICALWATER CASE -> EXTINGUISHED = NO WATER/NON FILLED //ECHEMICALCERAMIC CASE -> BREAKED
	Contained UMETA(DisplayName = "Contained"),
	Splashing UMETA(DisplayName = "Splashing")
};

UENUM(BlueprintType)
enum class EChemicalTransformation : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Burning UMETA(DisplayName = "Burning"),
	Drenching UMETA(DisplayName = "Drenching"),
	Staining UMETA(DisplayName = "Staining"),
	Erasing UMETA(DisplayName = "Erasing"),
	GivingIdentity UMETA(DisplayName = "GivingIdentity")
};