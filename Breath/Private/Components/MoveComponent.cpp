// Fill out your copyright notice in the Description page of Project Settings.

#include "MoveComponent.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values for this component's properties
UMoveComponent::UMoveComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UMoveComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UMoveComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UMoveComponent::MoveForward(float Value)
{
	ACharacter* Char = Cast<ACharacter>(GetOwner());

	FRotator CamRot = Char->GetControlRotation();
	CamRot.Pitch = 0.0f;
	FVector MoveDir = CamRot.Vector();
	Char->GetCharacterMovement()->AddInputVector(MoveDir * Value);
}

void UMoveComponent::MoveRight(float Value)
{
	ACharacter* Char = Cast<ACharacter>(GetOwner());

	FRotator CamRot = Char->GetControlRotation();
	CamRot.Pitch = 0.0f;
	FVector MoveDir = CamRot.RotateVector(FVector::RightVector);
	Char->GetCharacterMovement()->AddInputVector(MoveDir * Value);
}