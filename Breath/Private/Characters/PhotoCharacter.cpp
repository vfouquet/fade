// Fill out your copyright notice in the Description page of Project Settings.

#include "PhotoCharacter.h"
#include "Components/SkeletalMeshComponent.h"

// Sets default values
APhotoCharacter::APhotoCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bTickEvenWhenPaused = true;
}

// Called when the game starts or when spawned
void APhotoCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (auto* mesh = GetMesh())
		mesh->SetTickableWhenPaused(true);
}

// Called every frame
void APhotoCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void APhotoCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

