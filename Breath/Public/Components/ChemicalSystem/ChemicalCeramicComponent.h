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

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual EChemicalTransformation		getEffectiveEffect(EChemicalType const& otherType, EChemicalState const& otherState) const override;
	virtual EChemicalTransformation		getPotentialNextTransformation() const override;
	virtual	EChemicalState				getNextState(EChemicalTransformation const& transformation) const override;
};
