// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainPlayerController.generated.h"

class AMainCharacter;
class UMoveComponent;

/**
 * 
 */
UCLASS()
class BREATH_API AMainPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void SetupInputComponent() override;

	virtual void Possess(APawn* aPawn) override;
	virtual void UnPossess() override;

	void	MoveForward(float Value);
	void	MoveRight(float Value);

	void	RotateHorizontal(float Value);
	void	RotateVertical(float Value);

	void	Jump();

private:
	AMainCharacter*	MainCharacter;
	UMoveComponent*	MoveComponent;
};
