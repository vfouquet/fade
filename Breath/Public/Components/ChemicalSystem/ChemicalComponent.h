// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ChemicalTypes.h"
#include "ChemicalStateChanger.h"
#include "Containers/Map.h"

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ChemicalComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BREATH_API UChemicalComponent : public UActorComponent
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FChemicalStateChanged, EChemicalState, previous, EChemicalState, next);

public:
	// Sets default values for this component's properties
	UChemicalComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
		virtual void	OnActorOverlap(UPrimitiveComponent* overlapComp, AActor* OtherActor, UPrimitiveComponent* otherComp);
	UFUNCTION(BlueprintCallable)
		virtual void	OnActorEndOverlap(UPrimitiveComponent* overlapComp, AActor* OtherActor, UPrimitiveComponent* otherComp);

	EChemicalType const&	GetType() const { return type; }
	UFUNCTION(BlueprintCallable)
		EChemicalState const&	GetState() const { return state; }

public:
	UPROPERTY(BlueprintAssignable)
	FChemicalStateChanged	stateChangedDelegate;

protected:
	virtual EChemicalTransformation		getEffectiveEffect(EChemicalType const& otherType, EChemicalState const& otherState) const { return EChemicalTransformation::None; }
	virtual EChemicalTransformation		getPotentialNextTransformation() const { return EChemicalTransformation::None; }
	virtual	EChemicalState				getNextState(EChemicalTransformation const& transformation) const { return EChemicalState::None; }

protected:
	TMap<EChemicalTransformation, ChemicalStateChanger>	currentChangers;
	EChemicalType										type = EChemicalType::None;
	EChemicalState										state = EChemicalState::None;

private:
	void notifyChemicalStateChanged(EChemicalState previous, EChemicalState next);

private:
	ChemicalStateChanger&	addStateChanger(EChemicalTransformation transformation);
};
