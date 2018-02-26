// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ChemicalComponent.h"
#include "InteractableComponent.h"
#include "Components/PrimitiveComponent.h"
#include <Array.h>

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "IdentityZoneComponent.generated.h"

class AIdentityZoneManager;

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

	UFUNCTION()
	virtual void	OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult) {}
	UFUNCTION()
	virtual void	OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {}

protected:
	AIdentityZoneManager*	manager = nullptr;
};
