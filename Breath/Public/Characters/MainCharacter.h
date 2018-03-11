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
class UBoxClimbComponent;

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
	UFUNCTION(BlueprintCallable)
	void	EndThrow();
	void	Stick();
	void	Jump() override;
	bool	Climb();
	UFUNCTION(BlueprintCallable)
	void	EndClimb();

	void	OnDamage();
	void	Die(FVector impact = FVector::ZeroVector, FVector impactLoc = FVector::ZeroVector, FName boneName = NAME_None);

	void	SetWalkMode();
	void	SetJogMode();
	void	SetGodMode(bool value);

	void	SetRotatingLeft(bool const value) { rotatingLeft = value; }
	void	SetRotatingRight(bool const value) { rotatingRight = value; }
	void	SetPushingAxis(float const& value) { pushingAxis = value; }	
	UFUNCTION(BlueprintCallable)
	void	SetHeadRotation(FRotator value);
	UFUNCTION(BlueprintCallable)
	void	HeadLookAt(FVector lookAtLocation);
	UFUNCTION(BlueprintCallable)
	void	SetCustomSpeed(bool customSpeed, float newSpeed = 0.0f);
	UFUNCTION(BlueprintCallable)
	void	UnsetCustomSpeed();
	UFUNCTION(BlueprintCallable)
	void	BlockCharacter() { bBlocked = true; }
	UFUNCTION(BlueprintCallable)
	void	UnblockCharacter() { bBlocked = false; }
	void	EnableMovingHeavyObjectMode();
	void	DisableMovingHeavyObjectMode();
	void	SetThrowingObject(bool const value) { bThrowingObject = value; }
	void	SetHoldingObject(bool const value) { bHoldingObject = value; }

	UFUNCTION(BlueprintPure)
	bool	CanThrow() const;
	UFUNCTION(BlueprintPure)
	bool	IsHoldingObject() const { return bHoldingObject; }
	bool	IsInAir() const;
	UFUNCTION(BlueprintPure)
	bool	isDead() const { return bIsDead; }
	UFUNCTION(BlueprintPure)
	FRotator const GetHeadRotation() const { return headRotation; }

public:
	/*Roll isn't used*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Head")
	FRotator	MaxHeadRotationRange;
	UPROPERTY(EditAnywhere, Category = "Climb")
	UAnimMontage*	ClimbMontage = nullptr;
	UPROPERTY(EditAnywhere, Category = "Climb")
	TArray<FComponentReference>	climbBoxesReferences;
	/*Time to validate climb by walking**/
	UPROPERTY(EditAnywhere, Category = "Climb")
	float	ClimbAngleTolerence = 45.0f;
	UPROPERTY(BlueprintAssignable)
	FDeathDelegate	OnDie;
private:
	UFUNCTION()
	void	computeClimbableBoxes();
	UFUNCTION()
	void	endCharacterClimbSnap();

private:
	UPROPERTY(VisibleAnywhere)
	USpringArmComponent*				SpringArmComponent;
	UPlayerCameraComponent*				CameraComponent;
	UHoldComponent*						holdComponent = nullptr;  //REMOVE THIS WITH PUSH/PULL TEMP CALL
	UMainCharacterMovementComponent*	mainCharacterMovement = nullptr;

	FRotator	headRotation;

	FTimerHandle		climbSnapTimerHandle;
	FVector				centerSpineBoneOffset;

	bool						bCustomSpeedEnabled = false;
	bool						bBlocked = false;
	bool						bMovingHeavyObject = false;
	bool						bThrowingObject = false;
	bool						bHoldingObject = false;
	TArray<UBoxClimbComponent*>	climbBoxes;
	UBoxClimbComponent*			validClimbableBox = nullptr;
	bool						canClimb = false;
	bool						bIsDead = false;

	//PUSH/PULL INPUT FROM CONTROLLER
	bool						rotatingLeft = false;
	bool						rotatingRight = false;
	float						pushingAxis = 0.0f;
};
