// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MainCharacterMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class BREATH_API UMainCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	
	virtual bool CheckFall(const FFindFloorResult& OldFloor, const FHitResult& Hit, const FVector& Delta, const FVector& OldLocation, float remainingTime, float timeTick, int32 Iterations, bool bMustJump) override;	
	virtual FVector GetAirControl(float DeltaTime, float TickAirControl, const FVector& FallAcceleration) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement")
	float	CoyoteTime = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement")
	float	AirControlPower = 500.0f;
private:
	float	currentCoyoteTime = 0.0f;
};
