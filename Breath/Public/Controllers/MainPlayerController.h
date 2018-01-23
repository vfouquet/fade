// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainPlayerController.generated.h"

class AMainCharacter;
class USpringArmComponent;

/**
 * 
 */
UCLASS()
class BREATH_API AMainPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintPure)
	ACameraActor*	GetCameraActor();

	virtual void SetupInputComponent() override;

	virtual void Possess(APawn* aPawn) override;
	virtual void UnPossess() override;

	void	MoveForward(float Value);
	void	MoveRight(float Value);

	void	RotateHorizontal(float Value);
	void	RotateVertical(float Value);

	void	Jump();

	void	Action();
	void	BeginGrab();
	void	StopGrab();

	float		GetStickLength() const { return FVector2D(this->GetInputAxisValue("MoveRight"), GetInputAxisValue("MoveForward")).Size(); }
	float		GetInputAngle() const { return FMath::RadiansToDegrees(FMath::Atan2(GetInputAxisValue("MoveForward"), GetInputAxisValue("MoveRight"))); }
	FRotator	GetCameraRotation() const { return PlayerCameraManager->GetCameraRotation(); }

#if WITH_EDITOR
	void	DebugPauseEditor();
#endif

protected:
	UPROPERTY(VisibleAnywhere)
	ACameraActor*		CameraActor;
	UPROPERTY(EditAnywhere)
	TSubclassOf<class ACameraActor>	CameraActorClass;

private:
	AMainCharacter*			MainCharacter;
	USpringArmComponent*	SpringArmComponent;
};
