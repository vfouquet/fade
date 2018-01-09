// Fill out your copyright notice in the Description page of Project Settings.

#include "MainCharacter.h"

#include "HoldComponent.h"
#include "MainCharacterMovementComponent.h"

#include "GameFramework/SpringArmComponent.h"
#include "Cameras/PlayerCameraComponent.h"

// Sets default values
AMainCharacter::AMainCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UMainCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComponent->SetupAttachment(this->GetRootComponent());

#if WITH_EDITOR
	if (bIsThirdPersonCamera == true)
	{
		CameraComponent = CreateDefaultSubobject<UPlayerCameraComponent>(TEXT("PlayerCameraComponent"));
		CameraComponent->SetupAttachment(SpringArmComponent);
	}
#endif

}

// Called when the game starts or when spawned
void AMainCharacter::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AMainCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	auto* temp = FindComponentByClass<UPhysicsConstraintComponent>();
}

// Called to bind functionality to input
void AMainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AMainCharacter::RotateHorizontal(float Value)
{
	float Input = Value * 100.0f * GetWorld()->GetDeltaSeconds();
	AddControllerYawInput(Input);
}

void AMainCharacter::RotateVertical(float Value)
{
	float Input = Value * 100.0f * GetWorld()->GetDeltaSeconds();
	AddControllerPitchInput(Input);
}

void	AMainCharacter::Action()
{
	UHoldComponent*	holdComp = FindComponentByClass<UHoldComponent>();
	if (!holdComp)
		return;
	holdComp->Action();
}

void	AMainCharacter::BeginGrab()
{
	UHoldComponent*	holdComp = FindComponentByClass<UHoldComponent>();
	if (!holdComp)
		return;
	holdComp->Grab();
}

void	AMainCharacter::StopGrab()
{
	UHoldComponent*	holdComp = FindComponentByClass<UHoldComponent>();
	if (!holdComp)
		return;
	holdComp->StopGrab();
}

void	AMainCharacter::Throw()
{
	UHoldComponent*	holdComp = FindComponentByClass<UHoldComponent>();
	if (!holdComp)
		return;
	holdComp->Throw();
}

void	AMainCharacter::Stick()
{
	UHoldComponent* holdComp = FindComponentByClass<UHoldComponent>();
	if (!holdComp)
		return;
	holdComp->Stick();
}

void	AMainCharacter::Jump()
{
	UMoveComponent*	moveComp = FindComponentByClass<UMoveComponent>();
	if (moveComp)
	{
		if (moveComp->IsHoldingObject() || moveComp->IsMovingHeavyObject())
			return;
		if (moveComp->Climb())
			return;
		Super::Jump();
	}
}