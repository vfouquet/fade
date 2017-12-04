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
	
	void	BlockCharacter() { isBlocked = true; }
	void	UnblockCharacter() { isBlocked = false; }
	void	EnableMovingHeavyObjectMode() { isMovingHeavyObject = true; }
	void	DisableMovingHeavyObjectMode() { isMovingHeavyObject = false; }

	UPROPERTY(EditAnywhere, BLueprintReadWrite, Category = "Speed")
	float	WalkThreshold = 0.1f;
	UPROPERTY(EditAnywhere, BLueprintReadWrite, Category = "Speed")
	float	JogThreshold = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heavy Values")
	float	HeavyAngleTolerance = 5.0f;

private:
	
	FVector2D	currentInputValue;
	bool		isBlocked = false;
	bool		isMovingHeavyObject = false;
};
