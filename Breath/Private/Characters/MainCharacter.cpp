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
	AMainPlayerController*	mainController = GetMainPlayerController();

	if (bBlocked)
	{
		if ((holdComponent && holdComponent->IsMovingHeavyObject()) || mainController->GetStickLength() == 0.0f)
			return;
		float difference = GetCameraStickAngleDifference();
		float coeff = (difference >= 0.0f && difference < 180.0f) ? 1.0f : -1.0f;
		mainCharacterMovement->ProcessThrowRotation(DeltaTime * coeff);
		return;
	}

	if (canClimb && holdComponent && 
		!holdComponent->IsHoldingObject() && !holdComponent->IsMovingHeavyObject())
	{
		FVector vel = GetCharacterMovement()->Velocity;
		//isClimbAngleCorrect();
		if (vel.Size() > RunClimbVelocityThreshold)
			validateRunClimbCurrentTime += DeltaTime;
		else
			validateRunClimbCurrentTime = 0.0f;
	}
	else
		validateRunClimbCurrentTime = 0.0f;

	if (validateRunClimbCurrentTime >= RunClimbValue)
		Climb();

	if (holdComponent && holdComponent->IsMovingHeavyObject())
	{
		if (mainController->GetStickLength() == 0.0f)
			return;
		float difference = GetCameraStickAngleDifference();

		if (difference >= 180.0f - HeavyAngleTolerance && difference <= 180.0f + HeavyAngleTolerance)
			mainCharacterMovement->ProcessRotateHeavyObject(true, holdComponent->GetHoldingObjectMass());
		else if (difference >= 360.0f - HeavyAngleTolerance || difference <= HeavyAngleTolerance)
			mainCharacterMovement->ProcessRotateHeavyObject(false, holdComponent->GetHoldingObjectMass());
		else
			mainCharacterMovement->ProcessPushPull((difference >= 0.0f && difference < 180.0f), holdComponent->GetHoldingObjectMass(), holdComponent->GetHoldingObjectLocation());
	}
	else
	{
		if (mainController->GetStickLength() < JogStickThreshold)
			mainCharacterMovement->SetWalkMode();
		else
			mainCharacterMovement->SetJogMode();
	}
}

// Called to bind functionality to input
void AMainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void	AMainCharacter::MoveForward(float Value)
{
	if (bBlocked)
		return;
	if (holdComponent && !holdComponent->IsMovingHeavyObject())
	{
		if (Value <= WalkStickThreshold && Value >= -WalkStickThreshold)
			return;
		FRotator CamRot = ((AMainPlayerController*)Controller)->GetCameraRotation();
		CamRot.Pitch = 0.0f;
		FVector MoveDir = CamRot.Vector();
		if (Value < 0.0f)
			GetCharacterMovement()->AddInputVector(MoveDir * -1.0f);
		else
			GetCharacterMovement()->AddInputVector(MoveDir);
	}
}

void	AMainCharacter::MoveRight(float Value)
{
	if (bBlocked)
		return;

	if (holdComponent && !holdComponent->IsMovingHeavyObject())
	{
		if (Value <= WalkStickThreshold && Value >= -WalkStickThreshold)
			return;

		FRotator CamRot = ((AMainPlayerController*)Controller)->GetCameraRotation();
		CamRot.Pitch = 0.0f;
		FVector MoveDir = CamRot.RotateVector(FVector::RightVector);
		if (Value < 0.0f)
			GetCharacterMovement()->AddInputVector(MoveDir * -1.0f);
		else
			GetCharacterMovement()->AddInputVector(MoveDir);
	}
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
	
bool	AMainCharacter::IsInAir() const
{
	return mainCharacterMovement->IsFalling();
}

AMainPlayerController*	AMainCharacter::GetMainPlayerController() const
{ 
	return Cast<AMainPlayerController>(this->Controller); 
}

float	AMainCharacter::GetCameraStickAngleDifference() const
{
	AMainPlayerController* mainController = GetMainPlayerController();
	float	inputAngle = mainController->GetInputAngle();
	float cameraDiffAngle = GetMainPlayerController()->GetCameraRotation().Yaw - GetActorRotation().Yaw;

	inputAngle += 90.0f;
	if (inputAngle < 0.0f)
		inputAngle += 360.0f;

	if (cameraDiffAngle < 0.0f)
		cameraDiffAngle += 360.0f;

	float difference = inputAngle - cameraDiffAngle;

	if (difference < 0.0f)
		difference += 360.0f;
	return difference;
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