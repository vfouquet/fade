// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

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
	
	UFUNCTION(BlueprintPure)
	float	GetInputAngle() const { return FMath::RadiansToDegrees(FMath::Atan2(currentInputValue.Y, currentInputValue.X)); };
	UFUNCTION(BlueprintPure)
	float	GetCameraTargetDiffAngle() const;

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
	float	WalkThreshold = 0.1f;
	UPROPERTY(EditAnywhere, BLueprintReadWrite, Category = "Speed")
	float	JogThreshold = 0.5f;
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

private:
	bool		isBlocked = false;
	bool		isMovingHeavyObject = false;

	float		holdingObjectMass = 0.0f;
	FVector		holdingObjectLocation;
	
	FVector2D	currentInputValue;
};
