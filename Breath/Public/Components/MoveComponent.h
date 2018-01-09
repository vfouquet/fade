// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class UBoxClimbComponent;

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MoveComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BREATH_API UMoveComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMoveComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void	MoveForward(float Value);
	void	MoveRight(float Value);

	bool	Climb();

	UFUNCTION(BlueprintPure)
	float	GetInputAngle() const { return FMath::RadiansToDegrees(FMath::Atan2(currentInputValue.Y, currentInputValue.X)); };
	UFUNCTION(BlueprintPure)
	float	GetCameraTargetDiffAngle() const;
	bool	IsInAir() const;

	UFUNCTION(BlueprintCallable)
	void	BlockCharacter() { isBlocked = true; }
	UFUNCTION(BlueprintCallable)
	void	UnblockCharacter() { isBlocked = false; }
	UFUNCTION(BlueprintCallable)
	void	EnableMovingHeavyObjectMode();
	UFUNCTION(BlueprintCallable)
	void	DisableMovingHeavyObjectMode();
	UFUNCTION(BlueprintCallable)
	void	SetHoldingObjectLocationAndMass(FVector const& value, float const mass) { holdingObjectLocation = value; holdingObjectMass = mass; }

public:

	UPROPERTY(EditAnywhere, BLueprintReadWrite, Category = "Speed")
	float	WalkStickThreshold = 0.1f;
	UPROPERTY(EditAnywhere, BLueprintReadWrite, Category = "Speed")
	float	JogStickThreshold = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
	float	WalkSpeed = 300.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
	float	JogSpeed = 600.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heavy Values")
	float	HeavyAngleTolerance = 5.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heavy Values")
	float	RotationSpeed = 10.0f;
	/*Use the value for a mass threshold**/
	UPROPERTY(EditAnywhere, Category = "HeavyValues")
	float	MassGrabValues[3];
	/*Use the value for the multiplier**/
	UPROPERTY(EditAnywhere, Category = "HeavyValues")
	float	MassGrabMultipliers[3];
	
	UPROPERTY(EditAnywhere, Category = "Climb")
	TArray<FComponentReference>	climbBoxesReferences;
	/*Time to validate climb by walking**/
	UPROPERTY(EditAnywhere, Category = "Climb")
	float	RunClimbValue = 1.0f;
	/*Character velocity needed to validate a climb**/
	UPROPERTY(EditAnywhere, Category = "Climb")
	float	RunClimbVelocityThreshold = 200.0f;

private:
	void		setWalkMode();
	void		setJogMode();

	UFUNCTION()
	void	computeClimbableBoxes();
private:
	TArray<UBoxClimbComponent*>	climbBoxes;
	UBoxClimbComponent*			validClimbableBox = nullptr;
	float						validateRunClimbCurrentTime = 0.0f;
	bool						canClimb = false;

	bool						isBlocked = false;
	bool						isMovingHeavyObject = false;
	bool						isHoldingObject = false;

	float						holdingObjectMass = 0.0f;
	FVector						holdingObjectLocation;
	
	FVector2D					currentInputValue;
};
