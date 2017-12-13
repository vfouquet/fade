// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Components/SplineComponent.h"
#include "Cameras/RailCamera.h"

#include "CameraRailPoint.generated.h"

class ACameraRailManager;
class ACameraRailPoint;


USTRUCT()
struct FCameraRailSplinePoint : public FSplinePoint
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere)
	const ACameraRailPoint*	CameraRailPoint;

	FCameraRailSplinePoint()
	{
		CameraRailPoint = nullptr;
	}

	FCameraRailSplinePoint(ACameraRailPoint* CameraRailPoint)
	{
		CameraRailPoint = CameraRailPoint;
	}
};

UCLASS()
class BREATH_API ACameraRailPoint : public AActor
{
	GENERATED_BODY()

public:
	// Only used in editor
	UPROPERTY(VisibleAnywhere)
	ACameraRailManager*	CurrentCameraRailManager;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	TAssetPtr<ACameraRailPoint>	PreviousPoint;
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	TAssetPtr<ACameraRailPoint>	NextPoint;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FCameraRailSplinePoint	SplinePoint;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bInCameraSettings;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FCameraSettings	InCameraSettings;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOutCameraSettings;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FCameraSettings	OutCameraSettings;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Sets default values for this actor's properties
	ACameraRailPoint();

	UFUNCTION(BlueprintCallable)
	void	AddPreviousPointImpl();
	UFUNCTION(BlueprintCallable)
	void	AddNextPointImpl();
	UFUNCTION(BlueprintCallable)
	void	RemoveCurrentPointImpl();

#if WITH_EDITOR
	virtual void PreEditChange(UProperty* PropertyThatWillChange) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditMove(bool bFinished) override;
#endif

	virtual void Destroyed() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;
};

