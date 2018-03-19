// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Array.h"
#include "MainCharacterMovementComponent.h"

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MainCharacter.generated.h"

class USpringArmComponent;
class UPlayerCameraComponent;
class UHoldComponent;
class AMainPlayerController;

UCLASS()
class BREATH_API AMainCharacter : public ACharacter
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDeathDelegate);
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool	bIsThirdPersonCamera;

public:
	// Sets default values for this character's properties
	AMainCharacter(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void	Move(FVector InputVector);

	void	RotateHorizontal(float Value);
	void	RotateVertical(float Value);

	void	Action();
	void	BeginGrab();
	void	StopGrab();
	void	Throw();
	void	Stick();
	void	Jump(FVector direction = FVector::ZeroVector);
	bool	Climb();

	void	OnDamage();
	void	Die(FVector impact = FVector::ZeroVector, FVector impactLoc = FVector::ZeroVector, FName boneName = NAME_None);

	UFUNCTION(BlueprintCallable)
	void	HeadLookAt(FVector lookAtLocation);

	//NOTIFY CALLBACK
	UFUNCTION(BlueprintCallable)
	void	EndClimb();

	void	SetWalkMode();
	void	SetJogMode();
	void	SetGodMode(bool value);
	UFUNCTION(BlueprintCallable)
	void	SetHeadRotation(FRotator value);
	UFUNCTION(BlueprintCallable)
	void	SetCustomSpeed(bool customSpeed, float newSpeed = 0.0f);
	UFUNCTION(BlueprintCallable)
	void	UnsetCustomSpeed();

	void	SetRotatingLeft(bool const value) { rotatingLeft = value; }
	void	SetRotatingRight(bool const value) { rotatingRight = value; }
	void	SetPushingAxis(float const& value) { pushingAxis = value; }	
	
	UFUNCTION(BlueprintCallable)
	void	BlockCharacter() { bBlocked = true; }
	UFUNCTION(BlueprintCallable)
	void	UnblockCharacter() { bBlocked = false; }
	void	EnableMovingHeavyObjectMode();
	void	DisableMovingHeavyObjectMode();
	void	SetThrowingObject(bool const value) { bThrowingObject = value; }
	void	SetHoldingObject(bool const value) { bHoldingObject = value; }

	void	PlayLightGrabMontage(bool oneMeter = false) { PlayAnimMontage(oneMeter? LightGrabOneMeterAnim : LightGrabAnim); }
	void	PlayLightThrowMontage() { PlayAnimMontage(LightThrowAnim); }
	void	PlayHeavyGrabMontage() { PlayAnimMontage(HeavyGrabAnim); }
	void	PlayHeavyThrowMontage() { PlayAnimMontage(HeavyPushAnim); }
	void	StopLightGrabMontage() { StopAnimMontage(LightGrabAnim); }
	void	StopHeavyGrabMontage() { StopAnimMontage(HeavyGrabAnim); }

	UFUNCTION(BlueprintPure)
	bool			IsHeavyModeEnabled() const { return bMovingHeavyObject; }
	UFUNCTION(BlueprintPure)
	bool			IsRotatingLeft() const { return rotatingLeft; }
	UFUNCTION(BlueprintPure)
	bool			IsRotatingRight() const { return rotatingRight; }
	UFUNCTION(BlueprintPure)
	bool			CanThrow() const;
	UFUNCTION(BlueprintPure)
	bool			IsHoldingObject() const { return bHoldingObject; }
	bool			IsInAir() const;
	UFUNCTION(BlueprintPure)
	bool			isDead() const { return bIsDead; }
	UFUNCTION(BlueprintPure)
	FRotator const	GetHeadRotation() const { return headRotation; }
	UFUNCTION(BlueprintPure)
	FVector			GetTwoHandsLocation() const;

public:
	/*Roll isn't used*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Head")
	FRotator	MaxHeadRotationRange;
	/*Time to validate climb by walking**/
	UPROPERTY(EditAnywhere, Category = "Climb")
	float	ClimbAngleTolerence = 45.0f;
	
	UPROPERTY(BlueprintAssignable)
	FDeathDelegate	OnDie;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimMontage*	ClimbMontage = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimMontage*	LightGrabAnim = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimMontage*	LightGrabOneMeterAnim = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimMontage*	LightThrowAnim = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimMontage*	HeavyGrabAnim = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimMontage*	HeavyPushAnim = nullptr;

private:
	bool	climbTrace(FVector& outHitLocation, FVector& outNormal, FVector& outTopPoint);
	UFUNCTION()
	void	endCharacterClimbSnap();
public:
	UFUNCTION(BlueprintCallable)
	void	stopCurrentPlayingMontage();

private:
	UPROPERTY(VisibleAnywhere)
	USpringArmComponent*				SpringArmComponent;
	UPlayerCameraComponent*				CameraComponent;
	UHoldComponent*						holdComponent = nullptr;  //REMOVE THIS WITH PUSH/PULL TEMP CALL
	UMainCharacterMovementComponent*	mainCharacterMovement = nullptr;

	FTimerHandle		climbSnapTimerHandle;
	FVector				climbPointTarget;

	FRotator			headRotation;

	bool	bCustomSpeedEnabled = false;
	bool	bBlocked = false;
	bool	bMovingHeavyObject = false;
	bool	bThrowingObject = false;
	bool	bHoldingObject = false;
	bool	bIsDead = false;
	
	//PUSH/PULL INPUT FROM CONTROLLER
	bool	rotatingLeft = false;
	bool	rotatingRight = false;
	float	pushingAxis = 0.0f;

	//TEMP CLIMB TRICK SHIT
	bool	isClimbing = false;
	FVector	rootHipsOffset;
	FVector	beginClimbActorLocation;
};
