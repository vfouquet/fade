// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ChemicalSystem/ChemicalComponent.h"
#include "ChemicalWoodComponent.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "Chemical Wood", BlueprintSpawnableComponent))
class BREATH_API UChemicalWoodComponent : public UChemicalComponent
{
	GENERATED_BODY()
	
	UChemicalWoodComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	virtual EChemicalTransformation		getEffectiveEffect(EChemicalType const& otherType, EChemicalState const& otherState) const override;
	virtual EChemicalTransformation		getPotentialNextTransformation() const override;
	virtual	EChemicalState				getNextState(EChemicalTransformation const& transformation) const override;
};
