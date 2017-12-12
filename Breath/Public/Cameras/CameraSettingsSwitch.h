// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Cameras/CameraRailPoint.h"
#include "CameraSettingsSwitch.generated.h"

class ACameraRailManager;

UCLASS()
class BREATH_API ACameraSettingsSwitch : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TAssetPtr<ACameraRailManager>	CameraRailManager;

	UPROPERTY(EditAnywhere)
	bool	bInCameraSettings;
	UPROPERTY(EditAnywhere)
	FCameraSettings	InCameraSettings;
	UPROPERTY(EditAnywhere)
	bool	bOutCameraSettings;
	UPROPERTY(EditAnywhere)
	FCameraSettings	OutCameraSettings;

public:	
	// Sets default values for this actor's properties
	ACameraSettingsSwitch();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

private:
	float	SplineInputKey;

};
