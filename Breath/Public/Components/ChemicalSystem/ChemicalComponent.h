// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ChemicalTypes.h"
#include "ChemicalStateChanger.h"
#include "Containers/Map.h"

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ChemicalComponent.generated.h"


UCLASS( ClassGroup=(Custom))
class BREATH_API UChemicalComponent : public UActorComponent
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FChemicalStateChanged, EChemicalTransformation, transformation, EChemicalState, previous, EChemicalState, next);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FChemicalCollision, UChemicalComponent*, OtherChemical, EChemicalTransformation, EffectOnThis);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FChemicalEndCollision, UChemicalComponent*, OtherChemical);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FChemicalPercussion, UChemicalComponent*, OtherChemical);

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
	UFUNCTION()
	void	OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit );

	UFUNCTION(BlueprintCallable)
	/*Called when entering erase zone**/
	void	EraseIdentity();
	UFUNCTION(BlueprintCallable)
	/*Called when exiting erase zone**/
	void	GiveIdentity(EChemicalState	previousState);
	UFUNCTION(BlueprintCallable)
	void	AddHitComponent(UChemicalComponent* primComp);

	UFUNCTION(BlueprintPure)
	static UChemicalComponent*	FindAssociatedChemicalComponent(UPrimitiveComponent* referenceComponent);
	EChemicalType const&	GetType() const { return type; }
	UFUNCTION(BlueprintCallable)
	EChemicalState const&	GetState() const { return state; }
	UFUNCTION(BlueprintPure)
	UPrimitiveComponent* GetAssociatedComponent() const { return associatedComponent; }

	UFUNCTION(BlueprintCallable)
	void	SetState(EChemicalState const value) { state = value; }
	UFUNCTION(BlueprintCallable)
	void	SetType(EChemicalType const value) { type = value; }

public:
	UPROPERTY(BlueprintAssignable)
	FChemicalStateChanged	stateChangedDelegate;
	UPROPERTY(BlueprintAssignable)
	FChemicalCollision		chemicalCollisionEvent;
	UPROPERTY(BlueprintAssignable)
	FChemicalEndCollision	chemicalEndCollisionEvent;
	UPROPERTY(BlueprintAssignable)
	FChemicalPercussion		chemicalPercussionEvent;
	/**Does a incomplete transformation remain and can be resumed later or object state is reset*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transformations")
	bool	bPersistantTransformation = false;
	UPROPERTY(EditAnywhere, Category = "Transformations")
	FComponentReference	AssociatedComponent;
	static	float	WeightThresholdValue;
	static	float	SpeedThresholdValue;

protected:
	virtual	void						getStateChangedUselessTransformation(TArray<EChemicalTransformation>& returnValues, EChemicalTransformation previousTransformation) const {}
	virtual EChemicalTransformation		getEffectiveEffect(EChemicalType const& otherType, EChemicalState const& otherState) const { return EChemicalTransformation::None; }
	virtual EChemicalTransformation		getPotentialSelfNextTransformation() const { return EChemicalTransformation::None; }
	virtual	EChemicalState				getNextState(EChemicalTransformation const& transformation) const { return EChemicalState::None; }
	virtual ChemicalStateChanger&		addStateChanger(EChemicalTransformation transformation);
	virtual	bool						computePercussionBreakability(UPrimitiveComponent* other) { return false; }

protected:
	TMap<UChemicalComponent*, float>	hitChemicalComponents;

	TMap<EChemicalTransformation, ChemicalStateChanger>	currentChangers;
	UPrimitiveComponent*								associatedComponent = nullptr;
	EChemicalType										type = EChemicalType::None;
	EChemicalState										state = EChemicalState::None;

private:
	void	notifyChemicalStateChanged(EChemicalTransformation transformation, EChemicalState previous, EChemicalState next);
	void	addComponentToChangers(EChemicalTransformation transformation, UPrimitiveComponent* primComponent);
	void	removeComponentFromChangers(EChemicalTransformation transformation, UPrimitiveComponent* primComponent);
	void	updateImpact(UChemicalComponent* chemical, UPrimitiveComponent* primtive);
	void	refreshChangersWithCurrentInteractions();
	void	applyChemicalPhysics();
};
