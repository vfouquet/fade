// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IdentityZoneComponent.h"
#include "MemoryZoneComponent.generated.h"

/**
 * 
 */
UCLASS(meta=(DisplayName = "MemoryZone", BlueprintSpawnableComponent) )
class BREATH_API UMemoryZoneComponent : public UIdentityZoneComponent
{
	GENERATED_BODY()
	
public:
	virtual void	BeginPlay() override;
	virtual void	OnComponentDestroyed(bool bDestroyingHierarchy) override;
	UFUNCTION()
	virtual void	OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult) override;
	UFUNCTION()
	virtual void	OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;
	
	
};
