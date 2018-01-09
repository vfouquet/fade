// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MainCharacter.generated.h"

class USpringArmComponent;
class UPlayerCameraComponent;

UCLASS()
class BREATH_API AMainCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool	bIsThirdPersonCamera;

public:
	// Sets default values for this character's properties
	AMainCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void	RotateHorizontal(float Value);
	void	RotateVertical(float Value);
	
	void	Action();
	void	BeginGrab();
	void	StopGrab();
	void	Throw();
	void	Stick();
	void	Jump() override;

private:
	UPROPERTY(VisibleAnywhere)
	USpringArmComponent*		SpringArmComponent;
	UPlayerCameraComponent*		CameraComponent;

};
