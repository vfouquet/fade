// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "MainCharacterMovementComponent.generated.h"

class UInteractableComponent;

UENUM(BlueprintType)
enum class ELocomotionState : uint8
{
	Walking = 0 UMETA(DisplayName = "Walking"),
	Running = 1 UMETA(DisplayName = "Running"),
	Custom = 2 UMETA(DisplayName = "Custom")
};
/**
 * 
 */
UCLASS()
class BREATH_API UMainCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLocomotionStateDelegate, ELocomotionState, previousState, ELocomotionState, nextState);

public:
	virtual void	TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//virtual bool	CheckFall(const FFindFloorResult& OldFloor, const FHitResult& Hit, const FVector& Delta, const FVector& OldLocation, float remainingTime, float timeTick, int32 Iterations, bool bMustJump) override;	
	virtual bool	DoJump(bool bReplayingMoves) override;
	virtual void	SetPostLandedPhysics(const FHitResult& Hit) override;

	void	ProcessPushAndPull(float const& coeff, UInteractableComponent* holdingObject);
	void	ProcessRotateHeavyObject(float direction, UInteractableComponent* holdingObject, FVector holdingObjectLocation);
	void	ProcessThrowRotation(float coeff);

	void	EndJumping();

	void	SetWalkMode();
	void	SetJogMode();
	void	SetCustomSpeed(float customSpeed);
	void	SetJumpDirection(FVector value) { jumpDirection = value; }

	UFUNCTION(BlueprintPure)
	ELocomotionState	GetCurrentLocomotionState() const { return currentLocomotionState; }
	UFUNCTION(BlueprintPure)
	bool	IsJumping() const { return bIsJumping; }
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
	float	MoveRotationSpeed = 360.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
	float	FallingRotationSpeed = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
	float	RotateInterpSpeed = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
	float	PushPullInterpSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Push/Pull")
	float	PushPullSpeed = 200.f;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom Movement")
	//float	CoyoteTime = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forward Jump")
	float	LateralJumpForce = 300.0f;

	UPROPERTY(BlueprintAssignable)
	FLocomotionStateDelegate onLocomotionStateChanged;

private:
	void	updatePushAndPull();
	void	updateRotatePushAndPull();

private:
	UInteractableComponent*	holdingObject = nullptr;
	ELocomotionState		currentLocomotionState = ELocomotionState::Walking;
	FVector	jumpDirection;
	//float	currentCoyoteTime = 0.0f;
	FVector	lastOffsetLocation = FVector::ZeroVector;
	bool	bIsJumping = false;
	float	currentPushAndPullSpeed = 0.0f;
	float	currentRotateSpeed = 0.0f;
	float	pushAndPullTargetSpeed = 0.0f;
	float	rotateTargetSpeed = 0.0f;

protected:
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

};
