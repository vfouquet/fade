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

public:
	void	InitializeComponent() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Values")
	float	BreakByFireTime = 2.0f;

private:
	virtual EChemicalTransformation		getEffectiveEffect(EChemicalType const& otherType, EChemicalState const& otherState) const override;
	virtual	EChemicalState				getNextState(EChemicalTransformation const& transformation) const override;
	virtual ChemicalStateChanger&		addStateChanger(EChemicalTransformation transformation);
	virtual	bool						computePercussionBreakability(UPrimitiveComponent* other) override;
};
