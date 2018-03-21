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
	void	InitializeComponent() override;

	UFUNCTION(BlueprintPure)
	bool	IsInsideBody() const { return InsideBody; }
	UFUNCTION(BlueprintCallable)
	void	SetOutsideBody();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water properties")
	bool	InsideBody = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water properties")
	bool	AlreadyFilled = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water properties")
	float FillingTime = 0.1f;

private:
	virtual ChemicalStateChanger&		addStateChanger(EChemicalTransformation transformation);
	virtual EChemicalTransformation		getEffectiveEffect(EChemicalType const& otherType, EChemicalState const& otherState) const override;
	virtual	EChemicalState				getNextState(EChemicalTransformation const& transformation) const override;
};