// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ChemicalSystem/ChemicalComponent.h"
#include "ChemicalCeramicComponent.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "Chemical Ceramic", BlueprintSpawnableComponent))
class BREATH_API UChemicalCeramicComponent : public UChemicalComponent
{
	GENERATED_BODY()
	
	UChemicalCeramicComponent();

private:
	virtual	bool						computePercussionBreakability(UPrimitiveComponent* other) override;
};
