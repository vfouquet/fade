// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CameraZone.generated.h"

class UBoxComponent;
class APlayerCamera;
class UViewTargetComponent;

UCLASS()
class BREATH_API ACameraZone : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditInstanceOnly)
	AActor*	ViewTarget;

public:
	// Sets default values for this actor's properties
	ACameraZone();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;

private:
	UPROPERTY(VisibleAnywhere)
	UBoxComponent*	BoxComponent;

	AActor*			PlayerActor;
	APlayerCamera*	PlayerCamera;

};
