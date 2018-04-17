// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/DestructibleComponent.h"

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

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FBreakDelegate);

public:
	void	InitializeComponent() override;
	UFUNCTION(BlueprintCallable)
	void	Break() { swapTrick(); }

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Values")
	float	BreakByFireTime = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Break Values")
	float	HeightBreakThreshold = 400.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Break Values")
	float	OtherActorThreshold = 10000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swap Trick")
	bool	DoSwapTrick = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swap Trick")
	UClass*	ActorToSwap = nullptr;
	UPROPERTY(BlueprintAssignable)
	FComponentFractureSignature OnDestructibleFracture;

private:
	virtual EChemicalTransformation		getEffectiveEffect(EChemicalType const& otherType, EChemicalState const& otherState) const override;
	virtual	EChemicalState				getNextState(EChemicalTransformation const& transformation) const override;
	virtual ChemicalStateChanger&		addStateChanger(EChemicalTransformation transformation);
	virtual	bool						computePercussionBreakability(UPrimitiveComponent* other) override;
public:
	UFUNCTION()
	void	OnStateChanged(EChemicalTransformation tranform, EChemicalState previous, EChemicalState next);
	UFUNCTION(BlueprintCallable)
	void	swapTrick();
};
