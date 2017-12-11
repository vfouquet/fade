// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Cameras/CameraRailPoint.h"
#include "CameraRailManager.generated.h"

class USplineComponent;
class ACameraRailPoint;
class ARailCamera;

UCLASS()
class BREATH_API ACameraRailManager : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	ARailCamera*		RailCamera;
	UPROPERTY(VisibleAnywhere)
	AActor*				PlayerActor;



public:
	ACameraRailManager();

	float	GetCurrentInputKey();

	void	AttachCamera(ARailCamera* CameraToAttach);
	void	DetachCamera();

protected:
	virtual void BeginPlay() override;

public:
	virtual void PreEditChange(UProperty* PropertyThatWillChange) override;
	virtual void PostEditMove(bool bFinished) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	virtual void Tick(float DeltaSeconds) override;

	void	UpdateSpline();

	ACameraRailPoint*	GetCameraPointAtInputKey(float InputKey);

protected:
	UPROPERTY(VisibleAnywhere)
	USplineComponent*	SplineComponent;
	UPROPERTY(VisibleAnywhere)
	USplineComponent*	FlatSplineComponent;

	UPROPERTY(EditAnywhere)
	TAssetPtr<ACameraRailPoint>	RootPoint;

private:
	float	CurrentInputKey;
};
