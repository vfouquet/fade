// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraActor.h"
#include "PlayerCamera.generated.h"

//class ACharacter;
//class USceneComponent;

class USceneComponent;
class UTimelineComponent;
class USpringArmComponent;

/**
 * 
 */
UCLASS()
class BREATH_API APlayerCamera : public ACameraActor
{
	GENERATED_BODY()
	
public:
	APlayerCamera();
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable)
	void	AttachToTarget(AActor* NewTarget, bool bTeleport = false);

protected:
	UPROPERTY(VisibleAnywhere)
	AActor*				Target;
	UPROPERTY(VisibleAnywhere)
	USceneComponent*	ViewTarget;

	UPROPERTY(EditAnywhere)
	UCurveFloat*		TargetTransitionCurve;

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void	TargetTransitionCallback(float StepValue);
	UFUNCTION()
	void	TargetTransitionEndCallback();

private:
	UPROPERTY(VisibleAnywhere)
	USpringArmComponent*	SpringArmComponent;

	ACharacter*	PlayerCharacter;

	UTimelineComponent*	TargetTransitionTimeline;
	FTransform			TargetTransitionStartTransform;
};
