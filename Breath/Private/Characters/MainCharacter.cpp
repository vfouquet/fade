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
#include "Animation/AnimInstance.h"
#include "TimerManager.h"
#include "Kismet/KismetSystemLibrary.h"

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

	if (auto* mesh = GetMesh())
	{
		FVector boneLoc = mesh->GetBoneLocation("Maori_Hip_JNT");
		centerSpineBoneOffset = GetActorLocation() - boneLoc;
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
		//ADDITIONAL SECURITY TEST FOR PUSH/PULL SHOULD BE REMOVED
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

void	AMainCharacter::EndThrow()
{
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
	
void	AMainCharacter::SetHeadRotation(FRotator value)
{
	headRotation.Pitch = FMath::Clamp(value.Pitch, -MaxHeadRotationRange.Pitch, MaxHeadRotationRange.Pitch);
	headRotation.Yaw = FMath::Clamp(value.Yaw, -MaxHeadRotationRange.Yaw, MaxHeadRotationRange.Yaw);
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
}

bool	AMainCharacter::Climb()
{
	computeClimbableBoxes();
	if (!canClimb)
		return false;

	FHitResult	result;
	FVector	startTrace = GetActorLocation();
	startTrace.Z = validClimbableBox->GetComponentLocation().Z;
	FVector	endTrace = startTrace + GetActorForwardVector() * 70.0f;

	FCollisionQueryParams	params;
	params.AddIgnoredActor(this);
	bool res = GetWorld()->LineTraceSingleByChannel(result, startTrace, endTrace, ECollisionChannel::ECC_Visibility, params);
	if (res)
	{
		float angle = FMath::RadiansToDegrees(FMath::Acos(GetActorForwardVector() | (result.Normal * -1.0f)));
		if (angle > ClimbAngleTolerence)
			return false;

		FVector	newLoc = result.Location + result.Normal * 50.0f;
		newLoc.Z = GetActorLocation().Z;

		FLatentActionInfo	latentInfo;
		latentInfo.CallbackTarget = this;
		float interpTime =  (newLoc - GetActorLocation()).Size() / 100.0f;
		UKismetSystemLibrary::MoveComponentTo(GetCapsuleComponent(), newLoc, (result.Normal * -1.0f).Rotation(), true, true, interpTime, true, EMoveComponentAction::Type::Move, latentInfo);
		FTimerDelegate	del;
		del.BindUFunction(this, "endCharacterClimbSnap");
		GetWorldTimerManager().SetTimer(climbSnapTimerHandle, del, interpTime, false);
		BlockCharacter();
	}
	return true;
}
	
void	AMainCharacter::endCharacterClimbSnap()
{
	PlayAnimMontage(ClimbMontage);
}

void	AMainCharacter::EndClimb()
{
	UnblockCharacter();
	/*
	if (auto* mesh = GetMesh())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s"), *centerSpineBoneOffset.ToString());
		FVector tempLoc = mesh->GetBoneLocation("Maori_Hip_JNT");
		SetActorLocation(tempLoc + centerSpineBoneOffset);
	}
	*/
	UCapsuleComponent*	characterCapsule = FindComponentByClass<UCapsuleComponent>();
	if (!characterCapsule)
		return;
	SetActorLocation(validClimbableBox->GetClimbedLocation() + FVector::UpVector * (characterCapsule->GetScaledCapsuleHalfHeight() + 10.0f),
		false, nullptr, ETeleportType::TeleportPhysics);
}