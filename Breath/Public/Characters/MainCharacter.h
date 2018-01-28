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

	void	MoveForward(float Value);
	void	MoveRight(float Value);

	void	RotateHorizontal(float Value);
	void	RotateVertical(float Value);

	void	Action();
	void	BeginGrab();
	void	StopGrab();
	void	Throw();
	void	Stick();
	void	Jump() override;
	bool	Climb();

	void	BlockCharacter() { bBlocked = true; }
	void	UnblockCharacter() { bBlocked = false; }
	void	EnableMovingHeavyObjectMode() { mainCharacterMovement->bOrientRotationToMovement = false; }
	void	DisableMovingHeavyObjectMode() { mainCharacterMovement->bOrientRotationToMovement = true; }

	UFUNCTION(BlueprintPure)
	bool	CanThrow() const;
	bool	IsInAir() const;
	AMainPlayerController*	GetMainPlayerController() const;
	float	GetCameraStickAngleDifference() const;

public:
	UPROPERTY(EditAnywhere, BLueprintReadWrite, Category = "Speed")
	float	WalkStickThreshold = 0.1f;
	UPROPERTY(EditAnywhere, BLueprintReadWrite, Category = "Speed")
	float	JogStickThreshold = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heavy Values")
		float	HeavyAngleTolerance = 5.0f;
	UPROPERTY(EditAnywhere, Category = "Climb")
		TArray<FComponentReference>	climbBoxesReferences;
	/*Time to validate climb by walking**/
	UPROPERTY(EditAnywhere, Category = "Climb")
		float	RunClimbValue = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb")
		float	ClimbAngleTolerence = 45.0f;

private:
	UFUNCTION()
	void	computeClimbableBoxes();
	UFUNCTION(BlueprintPure)
	bool	isClimbAngleCorrect() const;

private:
	UPROPERTY(VisibleAnywhere)
	USpringArmComponent*				SpringArmComponent;
	UPlayerCameraComponent*				CameraComponent;
	UHoldComponent*						holdComponent = nullptr;
	UMainCharacterMovementComponent*	mainCharacterMovement = nullptr;

	bool						bBlocked = false;
	TArray<UBoxClimbComponent*>	climbBoxes;
	UBoxClimbComponent*			validClimbableBox = nullptr;
	float						validateRunClimbCurrentTime = 0.0f;
	bool						canClimb = false;
};
