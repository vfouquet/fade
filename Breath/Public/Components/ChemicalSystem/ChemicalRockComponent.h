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
public:
	void	InitializeComponent() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Values")
	float	stainedToLit = 3.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Values")
	float	litToBurning = 5.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Values")
	float	burningToNone = 4.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Oil Values")
	float	toStained = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Values")
	float	toDrenched = 0.1f;

private:
	bool	canBurn() const { return state == EChemicalState::Stained || state == EChemicalState::Lit || state == EChemicalState::Burning; }
	bool	canBeStained() const { return state == EChemicalState::None; }
	bool	canBeDrenched() const { return state == EChemicalState::Stained || state == EChemicalState::Lit || state == EChemicalState::Burning; }

private:
	virtual	void						getStateChangedUselessTransformation(TArray<EChemicalTransformation>& returnValues, EChemicalTransformation previousTransformation) const override;
	virtual EChemicalTransformation		getEffectiveEffect(EChemicalType const& otherType, EChemicalState const& otherState) const override;
	virtual EChemicalTransformation		getPotentialSelfNextTransformation() const override;
	virtual	EChemicalState				getNextState(EChemicalTransformation const& transformation) const override;
	virtual ChemicalStateChanger&		addStateChanger(EChemicalTransformation transformation);
};
