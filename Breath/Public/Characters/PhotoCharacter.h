// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PoseSnapshot.h"
#include "Camera/CameraComponent.h"

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

	void	MoveForward(float value);
	void	MoveRight(float value);
	void	RotateHorizontal(float value);
	void	RotateVertical(float value);

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

	UFUNCTION(BlueprintPure)
	float	GetCurrentAddedRollValue() const { return currentAddedRollValue; }
	UFUNCTION(BlueprintPure)
	FString	GetCurrentBodyPoseName() const { return currentBodyIndex < BodyPoses.Num() ? BodyPoses[currentBodyIndex].Name : "Error"; }
	UFUNCTION(BlueprintPure)
	FString	GetCurrentFacePoseName() const { return currentFaceIndex < FacePoses.Num() ? FacePoses[currentFaceIndex].Name : "Error"; }
	UFUNCTION(BlueprintPure)
	FPoseSnapshot&	GetSnapshotPose() { return lastPose; }
	UFUNCTION(BlueprintPure)
	UCameraComponent*	GetPhotoCamera() const { return photoCamera.IsValid() ? photoCamera.Get() : nullptr; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Poses")
	TArray<FPhotoPose>	BodyPoses;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Poses")
	TArray<FPhotoPose>	FacePoses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
	float	MoveSpeed = 100.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed")
	float	RotateSpeed = 100.0f;

private:
	float								currentAddedRollValue = 0.0f;
	FPoseSnapshot						lastPose;
	int									currentBodyIndex = 0;
	int									currentFaceIndex = 0;
	UAnimMontage*						lastPlayingBodyMontage = nullptr;
	UAnimMontage*						lastPlayingFaceMontage = nullptr;
	TWeakObjectPtr<UCameraComponent>	photoCamera;
};
