// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PoseSnapshot.h"

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PhotoCharacter.generated.h"

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

	UFUNCTION(BlueprintCallable)
	void	OverrideBasePose(bool const value) { bOverrideBasePose = value; }
	UFUNCTION(BlueprintCallable)
	void	OverrideFacePose(bool const value) { bOverrideFacePose = value; }

	UFUNCTION(BlueprintPure)
	FPoseSnapshot&	GetSnapshotPose() { return lastPose; }
	UFUNCTION(BlueprintPure)
	bool			IsOverridingBasePose() const { return bOverrideBasePose; }
	UFUNCTION(BlueprintPure)
	bool			IsOverridingFacePose() const { return bOverrideFacePose; }

private:
	FPoseSnapshot	lastPose;
	bool			bOverrideBasePose = false;
	bool			bOverrideFacePose = false;
};
