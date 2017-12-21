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
public:	
	UChemicalWoodComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Values")
	float	drenchToNormal = 3.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Values")
	float	normalToLit = 3.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Values")
	float	litToBurning = 5.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Values")
	float	burningToScorched = 4.0f;

private:
	bool	canBurn() const { return state == EChemicalState::None || state == EChemicalState::Lit || state == EChemicalState::Burning || state == EChemicalState::Stained; }
	bool	canBeStained() const { return state == EChemicalState::Drenched || state == EChemicalState::None; }

	virtual ChemicalStateChanger&		addStateChanger(EChemicalTransformation transformation);
	virtual EChemicalTransformation		getEffectiveEffect(EChemicalType const& otherType, EChemicalState const& otherState) const override;
	virtual EChemicalTransformation		getPotentialSelfNextTransformation() const override;
	virtual	EChemicalState				getNextState(EChemicalTransformation const& transformation) const override;
};
