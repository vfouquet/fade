// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ChemicalComponent.h"
#include "InteractableComponent.h"
#include "Components/PrimitiveComponent.h"
#include <Array.h>

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "IdentityZoneComponent.generated.h"

/**
 * 
 */
UCLASS()
class BREATH_API UIdentityZoneComponent : public USphereComponent
{
	GENERATED_BODY()
	
public:
	UIdentityZoneComponent() { PrimaryComponentTick.bCanEverTick = true; }

	virtual void	BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UFUNCTION()
	virtual void	OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult) {}
	UFUNCTION()
	virtual void	OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {}

protected:
	struct FErasedObjectProperties
	{
		TWeakObjectPtr<UPrimitiveComponent>		primitiveComponent = nullptr;
		TWeakObjectPtr<UChemicalComponent>		chemicalComponent = nullptr;
		TWeakObjectPtr<UInteractableComponent>	interactableComponent = nullptr;
		FVector								initialVelocity;
		EChemicalState						previousChemicalState;
		float								currentDecelerationTime = 0.0f;
		float								maxDeceleratingTime = 0.1f;
		bool								bWasSimulatingPhysics = false;
		bool								bDecelerating = false;
		int8								memoryZoneNbr = 0;
		int8								erasedZoneNbr = 0;
		float								lastTimeUpdated = 0.0f;
	};
	
	
	static FErasedObjectProperties&	createNewProperties(UPrimitiveComponent* primitiveComponent, float decelerationTime = 0.0f);
	static FErasedObjectProperties*	containsErasedObjectProperties(UPrimitiveComponent* reference, bool& foundSomething, int& outID);
	static void	updateObjectProperties(FErasedObjectProperties& properties);
	static void	updateObjectProperties(FErasedObjectProperties& properties, float decelerationTime);

protected:
	static TArray<FErasedObjectProperties>	affectedObjects;
};
