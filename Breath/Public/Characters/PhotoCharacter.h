// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PoseSnapshot.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PhotoCharacter.generated.h"

USTRUCT(BlueprintType)
struct FPhotoPose
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Values")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Values")
	UAnimMontage*	AssociatedMontage = nullptr;
};

UCLASS()
class BREATH_API APhotoCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APhotoCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void	SetSpeedCoef(float value) { speedCoef = value; }

	void	MoveForward(float value);
	void	MoveRight(float value);
	void	MoveUp(float value);
	void	RotateHorizontal(float value);
	void	RotateVertical(float value);

	UFUNCTION(BlueprintCallable)
	void	Toggle4KShot() { b4KShot = !b4KShot; }
	
	UFUNCTION(BlueprintCallable)
	void	HideCharacter(bool value);
	
	UFUNCTION(BlueprintCallable)
	void	PlayPreviousBodyPose();
	UFUNCTION(BlueprintCallable)
	void	PlayNextBodyPose();
	UFUNCTION(BlueprintCallable)
	void	PlayPreviousFacePose();
	UFUNCTION(BlueprintCallable)
	void	PlayNextFacePose();
	
	UFUNCTION(BlueprintCallable)
	void	IncreaseFOV();
	UFUNCTION(BlueprintCallable)
	void	DecreaseFOV();
	UFUNCTION(BlueprintCallable)
	void	IncreaseCameraRoll();
	UFUNCTION(BlueprintCallable)
	void	DecreaseCameraRoll();

	UFUNCTION(BlueprintCallable)
	void	IncreaseBrightness();
	UFUNCTION(BlueprintCallable)
	void	DecreaseBrightness();
	UFUNCTION(BlueprintCallable)
	void	ToggleDOF();
	UFUNCTION(BlueprintCallable)
	void	DecreaseDOFScale();
	UFUNCTION(BlueprintCallable)
	void	IncreaseDOFScale();
	UFUNCTION(BlueprintCallable)
	void	DecreaseDOFRegion();
	UFUNCTION(BlueprintCallable)
	void	IncreaseDOFRegion();
	UFUNCTION(BlueprintCallable)
	void	DecreaseVignetting();
	UFUNCTION(BlueprintCallable)
	void	IncreaseVignetting();

	UFUNCTION(BlueprintPure)
	bool	Is4KShotEnable() const { return b4KShot; }
	UFUNCTION(BlueprintPure)
	float	GetCurrentBrightnessValue() const { return currentBrightnessValue; }
	UFUNCTION(BlueprintPure)
	float	GetCurrentAddedRollValue() const { return currentAddedRollValue; }

	UFUNCTION(BlueprintPure)
	FString	GetCurrentBodyPoseName() const { return currentBodyIndex < BodyPoses.Num() ? BodyPoses[currentBodyIndex].Name : "Error"; }
	UFUNCTION(BlueprintPure)
	FString	GetCurrentFacePoseName() const { return currentFaceIndex < FacePoses.Num() ? FacePoses[currentFaceIndex].Name : "Error"; }
	UFUNCTION(BlueprintPure)
	FPoseSnapshot&	GetSnapshotPose() { return lastPose; }
	
	UFUNCTION(BlueprintPure)
	bool	IsDOFEnable() const { return bEnableDOF; }
	UFUNCTION(BlueprintPure)
	float	GetVignetting() const { return vignettingValue; }
	UFUNCTION(BlueprintPure)
	float	GETDOFScale() const { return dofScale; }
	UFUNCTION(BlueprintPure)
	float	GETDOFRegion() const { return dofRegion; }

	UFUNCTION(BlueprintPure)
	UCameraComponent*	GetPhotoCamera() const { return photoCamera.IsValid() ? photoCamera.Get() : nullptr; }

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Poses")
	TArray<FPhotoPose>	BodyPoses;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Poses")
	TArray<FPhotoPose>	FacePoses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
	float	MoveSpeed = 100.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
	float	RotateSpeed = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	FVector	MaxLateralOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
	FPostProcessSettings	CurrentPostProcessSettings;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
	FVector2D				DOFScaleRange = FVector2D(0.0f, 1.0f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
	FVector2D				DOFRegionRange = FVector2D(0.0f, 1000.0f);

private:
	bool								bEnableDOF = true;
	float								dofScale = 0.3f;
	float								dofRegion = 400.0f;
	bool								b4KShot = false;
	float								vignettingValue = 0.6f;
	float								speedCoef = 1.0f;
	float								currentBrightnessValue = 1.0f;
	float								currentAddedRollValue = 0.0f;
	FPoseSnapshot						lastPose;
	int									currentBodyIndex = 0;
	int									currentFaceIndex = 0;
	UAnimMontage*						lastPlayingBodyMontage = nullptr;
	UAnimMontage*						lastPlayingFaceMontage = nullptr;
	TWeakObjectPtr<UCameraComponent>	photoCamera;
	TWeakObjectPtr<USpringArmComponent>	springArm;

//INTERPVALUES
	bool	interpDOFScale = 0.0f;
	float	tempDOFScale = 0.0f;
	bool	interpDOFRegion = 0.0f;
	float	tempDOFRegion = 0.0f;
	bool	interpVignetting = 0.0f;
	float	tempVignettingValue = 0.0f;
	bool	interpBrightness = 0.0f;
	float	tempBrightnessValue = 0.0f;
	bool	interpRoll = 0.0f;
	float	tempRollValue = 0.0f;
};
