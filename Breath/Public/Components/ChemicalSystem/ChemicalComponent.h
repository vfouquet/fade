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

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FChemicalStateChanged, EChemicalTransformation, transformation, EChemicalState, previous, EChemicalState, next);

public:
	// Sets default values for this component's properties
	UChemicalComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION()
	void	OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	UFUNCTION()
	void	OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable)
	void	EraseIdentity();
	UFUNCTION(BlueprintCallable)
	void	GiveIdentity();

	EChemicalType const&	GetType() const { return type; }
	UFUNCTION(BlueprintCallable)
	EChemicalState const&	GetState() const { return state; }
	UFUNCTION(BlueprintCallable)
	void	SetState(EChemicalState const value) { state = value; }
	UFUNCTION(BlueprintCallable)
	void	SetType(EChemicalType const value) { type = value; }

public:
	UPROPERTY(BlueprintAssignable)
	FChemicalStateChanged	stateChangedDelegate;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transformations")
	bool	bPersistantTransformation = false;
	UPROPERTY(EditAnywhere, Category = "Transformations")
	FComponentReference	AssociatedComponent;

protected:
	virtual EChemicalTransformation		getEffectiveEffect(EChemicalType const& otherType, EChemicalState const& otherState) const { return EChemicalTransformation::None; }
	virtual EChemicalTransformation		getPotentialSelfNextTransformation() const { return EChemicalTransformation::None; }
	virtual	EChemicalState				getNextState(EChemicalTransformation const& transformation) const { return EChemicalState::None; }

protected:
	virtual ChemicalStateChanger&	addStateChanger(EChemicalTransformation transformation);

protected:
	TMap<EChemicalTransformation, ChemicalStateChanger>	currentChangers;
	EChemicalType										type = EChemicalType::None;
	EChemicalState										state = EChemicalState::None;

private:
	void notifyChemicalStateChanged(EChemicalTransformation transformation, EChemicalState previous, EChemicalState next);
};
