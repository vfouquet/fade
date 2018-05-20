// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpectatorPawn.h"
#include "MainSpectatorPawn.generated.h"

/**
 * 
 */
UCLASS()
class BREATH_API AMainSpectatorPawn : public ASpectatorPawn
{
	GENERATED_BODY()
	
public:
	void	BeginPlay() override;

	void	SetPhotoMode();
	void	SetBasicMode();

	void	MoveForward(float value);
	void	MoveRight(float value);
	void	RotateHorizontal(float value);
	void	RotateVertical(float value);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
	float	MoveSpeed = 100.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
	float	RotateSpeed = 100.0f;

protected:
	bool	bIsPhoto = false;
	float	SpringArmLength = 0.0f;
};
