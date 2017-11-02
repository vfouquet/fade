// Fill out your copyright notice in the Description page of Project Settings.

#include "CameraZone.h"

#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

#include "PlayerCamera.h"

// Sets default values
ACameraZone::ACameraZone()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	BoxComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
}

// Called when the game starts or when spawned
void ACameraZone::BeginPlay()
{
	Super::BeginPlay();
	
	PlayerActor = Cast<AActor>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
	PlayerCamera = Cast<APlayerCamera>(UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->GetViewTarget());

}

// Called every frame
void ACameraZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACameraZone::NotifyActorBeginOverlap(AActor* OtherActor)
{
	if (OtherActor == PlayerActor)
	{
		if (PlayerCamera != nullptr && ViewTarget != nullptr)
		{
			PlayerCamera->AttachToTarget(ViewTarget);
		}
	}
}

void ACameraZone::NotifyActorEndOverlap(AActor* OtherActor)
{
	if (OtherActor == PlayerActor)
	{
		if (PlayerCamera != nullptr)
		{
			PlayerCamera->AttachToTarget(PlayerActor);
		}
	}
}

