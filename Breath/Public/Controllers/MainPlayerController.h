// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UIWidgetControllerSupported.h"
#include "PhotoCharacter.h"
#include "AkAudio/Classes/AkAudioEvent.h"

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


	virtual void SetSpectatorPawn(class ASpectatorPawn* NewSpectatorPawn) override;

public:
	UFUNCTION(exec, BlueprintCallable)
	void	DebugMode(bool bValue);

	UFUNCTION(BlueprintPure)
	ACameraActor*	GetCameraActor();

	virtual void SetupInputComponent() override;

	virtual void Possess(APawn* aPawn) override;
	virtual void UnPossess() override;

	void	MoveForward(float Value);
	void	MoveRight(float Value);

	void	MovePhotoForward(float Value);
	void	MovePhotoRight(float Value);

	void	RotateHorizontal(float Value);
	void	RotateVertical(float Value);

	void	Pause();
	void	MenuUp();
	void	MenuDown();
	void	MenuLeft();
	void	MenuRight();
	void	MenuValidatePressed();
	void	MenuValidateReleased();
	void	MenuBackPressed();
	void	MenuBackReleased();
	void	MenuSpecialButton();

	void	Jump();

	void	Action();
	void	BeginGrab();
	void	StopGrab();

	UFUNCTION(BlueprintPure)
	AMainCharacter*	GetMainCharacter() const { return MainCharacter; }
	UFUNCTION(BlueprintPure)
	APhotoCharacter*	GetPhotoCharacter() const { return PhotoCharacter.IsValid()? PhotoCharacter.Get() : nullptr; }

	UFUNCTION(Exec)
	void	EnableGodMode();
	UFUNCTION(Exec)
	void	DisableGodMode();
	UFUNCTION(Exec, BlueprintCallable)
	void	PhotoMode(bool value, bool pauseGame = false);

	UFUNCTION(BlueprintImplementableEvent)
	void	OnPawnDeath();

	UFUNCTION(BlueprintCallable)
	void	SetCurrentWidget(UUIWidgetControllerSupported* umg) { currentUIWidget = umg; }

	float		GetInputAngle() const { return FMath::RadiansToDegrees(FMath::Atan2(GetInputAxisValue("MoveForward"), GetInputAxisValue("MoveRight"))); }
	FRotator	GetCameraRotation() const { return PlayerCameraManager->GetCameraRotation(); }

#if WITH_EDITOR
	void	DebugPauseEditor();
#endif

private:
	void	updateCharacterValues();

public:
	UPROPERTY(EditAnywhere, BLueprintReadWrite, Category = "Speed")
	float	WalkStickThreshold = 0.1f;
	UPROPERTY(EditAnywhere, BLueprintReadWrite, Category = "Speed")
	float	JogStickThreshold = 0.5f;
	UPROPERTY(EditAnywhere, BLueprintReadWrite, Category = "Jump")
	float	JumpStickThreshold = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heavy Values")
	float	HeavyAngleTolerance = 15.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climb")
	float	ClimbStickAngleTolerence = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Photo")
	TSubclassOf<class APhotoCharacter>	PhotoCharacterClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, CAtegory = "UI")
	TSubclassOf<class UUserWidget>	PauseWidgetSample;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	UAkAudioEvent*	PauseAllEvent = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	UAkAudioEvent*	ResumeAllEvent = nullptr;

protected:
	UPROPERTY(VisibleAnywhere)
	ACameraActor*		CameraActor;
	UPROPERTY(EditAnywhere)
	TSubclassOf<class ACameraActor>	CameraActorClass;

private:
	TWeakObjectPtr<APhotoCharacter>	PhotoCharacter = nullptr;
	AMainCharacter*					MainCharacter;
	USpringArmComponent*			SpringArmComponent;
	FVector2D						lastStickInput;
	bool							bIsTPS = false;
	bool							bIsPaused = false;

	TWeakObjectPtr<UUIWidgetControllerSupported>	currentUIWidget;
};
