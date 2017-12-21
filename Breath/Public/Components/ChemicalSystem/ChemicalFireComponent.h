// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ChemicalSystem/ChemicalComponent.h"
#include "ChemicalFireComponent.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "Chemical Fire", BlueprintSpawnableComponent))
class BREATH_API UChemicalFireComponent : public UChemicalComponent
{
	GENERATED_BODY()

public:
	UChemicalFireComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire properties")
		bool	bExtinguishedAtLaunch = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Values")
	float	toExtinguished = 0.5f;

private:
	virtual ChemicalStateChanger&		addStateChanger(EChemicalTransformation transformation);
	virtual EChemicalTransformation		getEffectiveEffect(EChemicalType const& otherType, EChemicalState const& otherState) const override;
	virtual EChemicalTransformation		getPotentialSelfNextTransformation() const override;
	virtual	EChemicalState				getNextState(EChemicalTransformation const& transformation) const override;
};