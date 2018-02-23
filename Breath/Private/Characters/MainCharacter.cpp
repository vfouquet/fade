// Fill out your copyright notice in the Description page of Project Settings.

#include "MainCharacter.h"

#include "HoldComponent.h"
#include "MainCharacterMovementComponent.h"
#include "CharacterHealthComponent.h"

#include "GameFramework/SpringArmComponent.h"
#include "Cameras/PlayerCameraComponent.h"
#include "MainPlayerController.h"
#include "Camera/CameraActor.h"
#include "BoxClimbComponent.h"
#include "Components/SkeletalMeshComponent.h"


// Sets default values
AMainCharacter::AMainCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UMainCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComponent->SetupAttachment(this->GetRootComponent());

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

#if WITH_EDITOR
	if (bIsThirdPersonCamera == true)
	{
		CameraComponent = NewObject<UPlayerCameraComponent>(this, UPlayerCameraComponent::StaticClass());
		CameraComponent->SetupAttachment(SpringArmComponent);
	}
#endif
}

// Called every frame
void AMainCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bBlocked)
		return;	

	if (bHoldingObject || bMovingHeavyObject || bThrowingObject)
	{
		bool ascending = false;
		if (mainCharacterMovement->IsFalling(ascending) && !ascending)
			OnDamage(); //DO THAT SHIT TO JUST TO RELEASE BECAUSE IT GOT THE SAME CONSEQUENCES THAN DAMAGE
	}

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
			if (rotatingLeft && holdComponent->CanRotateLeft(GetActorForwardVector()))
				mainCharacterMovement->ProcessRotateHeavyObject(false, holdComponent->GetHoldingObjectMass(), holdComponent->GetHoldingObjectLocation());
			else if (rotatingRight && holdComponent->CanRotateRight(GetActorForwardVector()))
				mainCharacterMovement->ProcessRotateHeavyObject(true, holdComponent->GetHoldingObjectMass(), holdComponent->GetHoldingObjectLocation());
			else if (!FMath::IsNearlyZero(pushingAxis))
			{
				if (pushingAxis == 1.0f && holdComponent->CanPushForward(GetActorForwardVector()))
					mainCharacterMovement->ProcessPushAndPull(pushingAxis, holdComponent->GetHoldingObjectMass());
				else if (pushingAxis == -1.0f)
					mainCharacterMovement->ProcessPushAndPull(pushingAxis, holdComponent->GetHoldingObjectMass());
			}
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

void	AMainCharacter::OnDamage()
{
	if (holdComponent)
		holdComponent->UniversalRelease();
}

void	AMainCharacter::Die(FVector impact, FVector impactLocation, FName boneName)
{
	bIsDead = true;
	OnDie.Broadcast();
	BlockCharacter();
	USkeletalMeshComponent* mesh = GetMesh();
	if (!mesh)
		return;
	mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	mesh->SetSimulatePhysics(true);
	mesh->WakeAllRigidBodies();
	if (boneName != NAME_None)
		mesh->AddImpulse(impact, boneName);
	else
		mesh->AddImpulseAtLocation(impact, impactLocation);
}

bool	AMainCharacter::CanThrow() const
{
	return bHoldingObject || bMovingHeavyObject;
}

void	AMainCharacter::SetWalkMode()
{
	if (!bCustomSpeedEnabled)
		mainCharacterMovement->SetWalkMode();
}

void	AMainCharacter::SetJogMode()
{
	if (!bCustomSpeedEnabled)
		mainCharacterMovement->SetJogMode();
}

void	AMainCharacter::SetGodMode(bool value)
{
	UCharacterHealthComponent* healthComp = FindComponentByClass<UCharacterHealthComponent>();
	if (healthComp)
		healthComp->SetGodMode(value);
}

void	AMainCharacter::SetCustomSpeed(bool customSpeed, float newSpeed)
{
	bCustomSpeedEnabled = true;
	mainCharacterMovement->MaxWalkSpeed = customSpeed? newSpeed : mainCharacterMovement->WalkSpeed;
}

void	AMainCharacter::UnsetCustomSpeed()
{
	bCustomSpeedEnabled = false;
	mainCharacterMovement->SetWalkMode();
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
	bool ascending = false;
	return mainCharacterMovement->IsFalling(ascending);
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