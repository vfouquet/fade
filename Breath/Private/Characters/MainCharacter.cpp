// Fill out your copyright notice in the Description page of Project Settings.

#include "MainCharacter.h"

#include "HoldComponent.h"
#include "MainCharacterMovementComponent.h"

#include "GameFramework/SpringArmComponent.h"
#include "Cameras/PlayerCameraComponent.h"
#include "MainPlayerController.h"
#include "Camera/CameraActor.h"
#include "BoxClimbComponent.h"

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
	holdComponent = FindComponentByClass<UHoldComponent>();
	mainCharacterMovement = Cast<UMainCharacterMovementComponent>(GetCharacterMovement());

	for (auto&& climbBoxesReference : climbBoxesReferences)
	{
		UBoxClimbComponent*	tempBox = Cast<UBoxClimbComponent>(climbBoxesReference.GetComponent(this));
		if (tempBox)
		{
			climbBoxes.Add(tempBox);
			FScriptDelegate	beginOverlapDel;
			beginOverlapDel.BindUFunction(this, "computeClimbableBoxes");
			tempBox->boxClimbOverlap.Add(beginOverlapDel);
			tempBox->boxClimbEndOverlap.Add(beginOverlapDel);
		}
	}

}

// Called every frame
void AMainCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bBlocked)
		return;	

	if (bHoldingObject && bThrowingObject)
	{
		if (rotatingLeft)
			mainCharacterMovement->ProcessThrowRotation(DeltaTime * -1.0f);
		else if (rotatingRight)
			mainCharacterMovement->ProcessThrowRotation(DeltaTime * 1.0f);
	}
	else if (bMovingHeavyObject)
	{
		if (holdComponent)
		{
			if (rotatingLeft)
				mainCharacterMovement->ProcessRotateHeavyObject(false, holdComponent->GetHoldingObjectMass(), holdComponent->GetHoldingObjectLocation());
			else if (rotatingRight)
				mainCharacterMovement->ProcessRotateHeavyObject(true, holdComponent->GetHoldingObjectMass(), holdComponent->GetHoldingObjectLocation());
			else if (!FMath::IsNearlyZero(pushingAxis))
				mainCharacterMovement->ProcessPushAndPull(pushingAxis, holdComponent->GetHoldingObjectMass());
		}
	}
	else if (canClimb && !bHoldingObject)
	{
		FVector vel = GetCharacterMovement()->Velocity;
		//isClimbAngleCorrect();
		if (vel.Size() > 0.0f)
			validateRunClimbCurrentTime += DeltaTime;
		else
			validateRunClimbCurrentTime = 0.0f;
	}
	else
		validateRunClimbCurrentTime = 0.0f;

	if (validateRunClimbCurrentTime >= RunClimbValue)
		Climb();
}

// Called to bind functionality to input
void AMainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void	AMainCharacter::Move(FVector Value)
{
	if (bBlocked || bMovingHeavyObject || bThrowingObject)
		return;
	if (!bMovingHeavyObject)
		GetCharacterMovement()->AddInputVector(Value);
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
	if (!holdComponent)
		return;
	holdComponent->Action();
}

void	AMainCharacter::BeginGrab()
{
	if (!holdComponent)
		return;
	holdComponent->Grab();
}

void	AMainCharacter::StopGrab()
{
	if (!holdComponent)
		return;
	holdComponent->StopGrab();
}

void	AMainCharacter::Throw()
{
	if (!holdComponent)
		return;
	holdComponent->Throw();
}

void	AMainCharacter::Stick()
{
	if (!holdComponent)
		return;
	holdComponent->Stick();
}

void	AMainCharacter::Jump()
{
	if (Climb())
		return;
	Super::Jump();
}

bool	AMainCharacter::CanThrow() const
{
	return holdComponent && (holdComponent->IsHoldingObject() || holdComponent->IsMovingHeavyObject());
}
	
void	AMainCharacter::EnableMovingHeavyObjectMode() 
{ 
	mainCharacterMovement->bOrientRotationToMovement = false;
	bMovingHeavyObject = true;
}

void	AMainCharacter::DisableMovingHeavyObjectMode()
{ 
	mainCharacterMovement->bOrientRotationToMovement = true; 
	bMovingHeavyObject = false;
}

bool	AMainCharacter::IsInAir() const
{
	return mainCharacterMovement->IsFalling();
}

void	AMainCharacter::computeClimbableBoxes()
{
	validClimbableBox = nullptr;
	canClimb = false;

	for (auto& climbBox : climbBoxes)
	{
		if (climbBox->IsOverlappingClimbingSurface() && !validClimbableBox)
			validClimbableBox = climbBox;
		else if (climbBox->IsOverlappingOthers())
			break;

	}
	if (validClimbableBox && validClimbableBox->CheckSpaceOver())
		canClimb = true;
	else
		validateRunClimbCurrentTime = 0.0f;
}

bool	AMainCharacter::isClimbAngleCorrect() const
{
	if (!validClimbableBox)
		return false;
	FHitResult	result;
	FCollisionQueryParams	queryParams;
	queryParams.AddIgnoredActor(GetOwner());
	if (!GetWorld()->LineTraceSingleByChannel(result, GetOwner()->GetActorLocation(), GetOwner()->GetActorForwardVector() * 100.0f, ECollisionChannel::ECC_WorldDynamic, queryParams))
		return false;
	FVector	character = GetOwner()->GetActorForwardVector() * -1.0f;
	character.Normalize();
	float test = FMath::RadiansToDegrees(FVector::DotProduct(character, result.Normal));
	UE_LOG(LogTemp, Warning, TEXT("Diff : %f"), test);
	return true;
}

bool	AMainCharacter::Climb()
{
	computeClimbableBoxes();
	if (!canClimb)
		return false;

	//BEGIN SNAP + BLOCK INPUT
	UCapsuleComponent*	characterCapsule = FindComponentByClass<UCapsuleComponent>();
	if (!characterCapsule)
		return false;
	SetActorLocation(validClimbableBox->GetClimbedLocation() + FVector::UpVector * (characterCapsule->GetScaledCapsuleHalfHeight() + 10.0f),
		false, nullptr, ETeleportType::TeleportPhysics);
	return true;
}