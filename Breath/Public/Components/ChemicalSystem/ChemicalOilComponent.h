// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ChemicalSystem/ChemicalComponent.h"
#include "ChemicalOilComponent.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "Chemical Oil", BlueprintSpawnableComponent))
class BREATH_API UChemicalOilComponent : public UChemicalComponent
{
	GENERATED_BODY()
	
public:
	void	InitializeComponent() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Values")
	float	toDestroy = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Values")
	float	noneToLit = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Values")
	float	litToBurning = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Values")
	float	burningToDestroy = 2.0f;

private:
	bool	canBurn() const { return state == EChemicalState::None || state == EChemicalState::Lit ||
			state == EChemicalState::Burning || state == EChemicalState::Stained;}
	bool	canBeStained() const { return false; }
	bool	canBeDrenched() const { return state == EChemicalState::None || state == EChemicalState::Lit || state == EChemicalState::Burning;}

private:
	virtual	void						getStateChangedUselessTransformation(TArray<EChemicalTransformation>& returnValues, EChemicalTransformation previousTransformation) const override;
	virtual EChemicalTransformation		getEffectiveEffect(EChemicalType const& otherType, EChemicalState const& otherState) const override;
	virtual EChemicalTransformation		getPotentialSelfNextTransformation() const override;
	virtual	EChemicalState				getNextState(EChemicalTransformation const& transformation) const override;
	virtual ChemicalStateChanger&		addStateChanger(EChemicalTransformation transformation);
};
