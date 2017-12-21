// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ChemicalSystem/ChemicalComponent.h"
#include "ChemicalWaterComponent.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "Chemical Water", BlueprintSpawnableComponent))
class BREATH_API UChemicalWaterComponent : public UChemicalComponent
{
	GENERATED_BODY()

public:
	UChemicalWaterComponent();

	virtual EChemicalTransformation		getEffectiveEffect(EChemicalType const& otherType, EChemicalState const& otherState) const override;
	virtual EChemicalTransformation		getPotentialSelfNextTransformation() const override;
	virtual	EChemicalState				getNextState(EChemicalTransformation const& transformation) const override;
};
