// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraActor.h"
#include "RailCamera.generated.h"


class USpringArmComponent;
class ACameraRailManager;
class UTimelineComponent;

USTRUCT()
struct FCameraInterestPoint
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere)
	TAssetPtr<AActor>	Actor;
	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.f, ClampMax = 1.f))
	float				Weight;
};

USTRUCT(BlueprintType)
struct FCameraSettings
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere)
	float	CameraSpeed; // interp
	UPROPERTY(EditAnywhere)
	float	CameraRotationSpeed; // interp
	UPROPERTY(EditAnywhere)
	float	CameraRailOffset;
	UPROPERTY(EditAnywhere, meta = (ClampMin = 0.f))
	float	CameraRailDistanceTolerance;
	UPROPERTY(EditAnywhere)
	float	CameraDistanceOffset;
	UPROPERTY(EditAnywhere)
	float	CameraDistanceOffsetSpeed; // interp
	UPROPERTY(EditAnywhere)
	float	CameraHeightOffset;
	UPROPERTY(EditAnywhere)
	float	CameraHeightOffsetSpeed; // interp
	UPROPERTY(EditAnywhere)
	TArray<FCameraInterestPoint>	InterestPoints;


	FCameraSettings()
	{
		CameraSpeed = 400.f;
		CameraRotationSpeed = 400.0f;
		CameraRailOffset = 0.0f;
		CameraRailDistanceTolerance = 0.0f;
		CameraDistanceOffset = 0.0f;
		CameraDistanceOffsetSpeed = 100.f;
		InterestPoints.Empty();
	}
};

UCLASS()
class BREATH_API ARailCamera : public ACameraActor
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TAssetPtr<ACameraRailManager>	CurrentRailManager;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FCameraSettings	PreviousCameraSettings;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FCameraSettings	CameraSettings;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FCameraSettings	CurrentCameraSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector	TargetRelativeLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float	SpeedInterpTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float	RotationSpeedInterpTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float	DistanceSpeedInterpTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float	HeightSpeedInterpTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float	RailOffsetInterpTime;

public:
	ARailCamera(const FObjectInitializer& ObjectInitializer);

	USceneComponent*	GetCameraArm();

	void UpdateCamera(float DeltaSeconds);
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	UFUNCTION(BlueprintCallable)
	void	AttachToRail(ACameraRailManager* CameraRailManager);
	UFUNCTION(BlueprintCallable)
	void	AttachToRailWithPlayer(ACameraRailManager* CameraRailManager, AActor* PlayerActor, bool bTeleport = false);
	UFUNCTION(BlueprintCallable)
	void	ChangePlayer(AActor* PlayerActor, bool bTeleport = false);

	UFUNCTION(BlueprintCallable)
	void	ChangeSettings(const FCameraSettings& Settings);

protected:
	UPROPERTY(VisibleAnywhere)
	USceneComponent*	CameraArm;

protected:
	virtual void BeginPlay() override;

private:
	float	SpeedInterpAlpha;
	float	RotationSpeedInterpAlpha;
	float	DistanceSpeedInterpAlpha;
	float	HeightSpeedInterpAlpha;
	float	RailOffsetInterpAlpha;
};
