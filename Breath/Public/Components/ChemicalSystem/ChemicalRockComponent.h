// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ChemicalSystem/ChemicalComponent.h"
#include "ChemicalRockComponent.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "Chemical Rock", BlueprintSpawnableComponent))
class BREATH_API UChemicalRockComponent : public UChemicalComponent
{
	GENERATED_BODY()
	
	UChemicalRockComponent();

	virtual EChemicalTransformation		getEffectiveEffect(EChemicalType const& otherType, EChemicalState const& otherState) const override;
	virtual EChemicalTransformation		getPotentialSelfNextTransformation() const override;
	virtual	EChemicalState				getNextState(EChemicalTransformation const& transformation) const override;
};
