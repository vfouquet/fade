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
	float	CameraSpeed;
	UPROPERTY(EditAnywhere)
	float	CameraRotationSpeed;
	UPROPERTY(EditAnywhere)
	float	CameraRailOffset;
	UPROPERTY(EditAnywhere)
	float	CameraDistanceOffset;
	UPROPERTY(EditAnywhere)
	float	CameraDistanceOffsetSpeed;
	UPROPERTY(EditAnywhere)
	TArray<FCameraInterestPoint>	InterestPoints;


	FCameraSettings()
	{
		CameraSpeed = 0.40f;
		CameraRotationSpeed = 400.0f;
		CameraRailOffset = 0.0f;
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
	FCameraSettings	CameraSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector	TargetRelativeLocation;

public:
	ARailCamera(const FObjectInitializer& ObjectInitializer);

	USceneComponent*	GetCameraArm();

	virtual void Tick(float DeltaSeconds) override;
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	UFUNCTION(BlueprintCallable)
	void	AttachToRail(ACameraRailManager* CameraRailManager);

protected:
	UPROPERTY(VisibleAnywhere)
	USceneComponent*	CameraArm;

protected:
	virtual void BeginPlay() override;

};
