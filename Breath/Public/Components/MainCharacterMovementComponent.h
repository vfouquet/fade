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
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual bool CheckFall(const FFindFloorResult& OldFloor, const FHitResult& Hit, const FVector& Delta, const FVector& OldLocation, float remainingTime, float timeTick, int32 Iterations, bool bMustJump) override;	
	virtual FVector GetAirControl(float DeltaTime, float TickAirControl, const FVector& FallAcceleration) override;
	virtual bool DoJump(bool bReplayingMoves) override;

	void	ProcessPushAndPull(float const& coeff, float holdingObjectMass);
	void	ProcessRotateHeavyObject(bool direction, float holdingObjectMass, FVector holdingObjectLocation);
	void	ProcessThrowRotation(float coeff);

	void	SetWalkMode() { MaxWalkSpeed = WalkSpeed; }
	void	SetJogMode() { MaxWalkSpeed = JogSpeed; }
	void	SetJumpDirection(FVector value) { jumpDirection = value; }

	bool	IsFalling(bool& ascending);
	FVector const& 	GetLastMovementOffset() const { return lastOffsetLocation; }

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
	float	WalkSpeed = 300.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
	float	JogSpeed = 600.0f;
	/*Use the value for a mass threshold**/
	UPROPERTY(EditAnywhere, Category = "HeavyValues")
	float	MassGrabValues[3];
	/*Use the value for the multiplier**/
	UPROPERTY(EditAnywhere, Category = "HeavyValues")
	float	MassGrabMultipliers[3];
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heavy Values")
	float	RotationSpeed = 10.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throw")
	float	ThrowRotationSpeed = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement")
	float	CoyoteTime = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement")
	float	AirControlPower = 500.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forward Jump")
	float	ForwardJumpAngle = 45.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forward Jump")
	float	ForwardJumpForce = 450.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forward Jump")
	float	LateralJumpForce = 300.0f;

private:
	FVector	jumpDirection;
	float	currentCoyoteTime = 0.0f;
	FVector	lastOffsetLocation = FVector::ZeroVector;
};
