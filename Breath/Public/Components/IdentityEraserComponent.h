// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <Array.h>

#include "Components/PrimitiveComponent.h"
#include "ChemicalComponent.h"

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "IdentityEraserComponent.generated.h"

UCLASS(meta=(DisplayName = "IdentityEraser", BlueprintSpawnableComponent) )
class BREATH_API UIdentityEraserComponent : public USphereComponent
{
	GENERATED_BODY()

	struct FErasedObjectProperties
	{
		TWeakObjectPtr<UPrimitiveComponent>	primitiveComponent = nullptr;
		TWeakObjectPtr<UChemicalComponent>	chemicalComponent = nullptr;
		FVector								initialVelocity;
		EChemicalState						previousChemicalState;
		float								currentDecelerationTime = 0.0f;
		bool								wasSimulatingPhysics = false;
		bool								decelerating = false;
	};
public:	
	// Sets default values for this component's properties
	UIdentityEraserComponent();
	virtual void	BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	UFUNCTION()
	void	OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	UFUNCTION()
	void	OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erased object properties")
	float	DecelerationTime = 0.1f;
private:
	TArray<FErasedObjectProperties>	erasedObjects;
};
