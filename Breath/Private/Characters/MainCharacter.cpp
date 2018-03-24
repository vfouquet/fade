// Fill out your copyright notice in the Description page of Project Settings.

#include "MainCharacter.h"

#include "HoldComponent.h"
#include "MainCharacterMovementComponent.h"
#include "CharacterHealthComponent.h"

#include "GameFramework/SpringArmComponent.h"
#include "Cameras/PlayerCameraComponent.h"
#include "MainPlayerController.h"
#include "Camera/CameraActor.h"
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

	//TEMP CLIMB TRICK
	if (auto* mesh = GetMesh())
		rootHipsOffset = mesh->GetBoneLocation("Maori_Hip_JNT") - GetActorLocation();

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

	//CLIMB TRICK
	/*
	if (isClimbing)
	{
		FVector translation = GetMesh()->GetBoneLocation("Maori_Hip_JNT") - rootHipsOffset - GetActorLocation();
		FVector newLoc = beginClimbActorLocation + translation;
		SetActorLocation(newLoc);
		GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -92.0f) - translation);
	}
	*/

	//updateClimbAnimationTranslation();

	if (bBlocked)
		return;	

	if (bHoldingObject || bMovingHeavyObject || bThrowingObject)
	{
		bool ascending = false;
		if (mainCharacterMovement->IsFalling(ascending) && !ascending)
		{
			if (holdComponent)
				holdComponent->UniversalRelease();
		}
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
			if (rotatingLeft)// && holdComponent->CanRotateLeft(GetActorForwardVector()))
				mainCharacterMovement->ProcessRotateHeavyObject(false, holdComponent->GetHoldingObjectMass(), holdComponent->GetHoldingObjectLocation());
			else if (rotatingRight)// && holdComponent->CanRotateRight(GetActorForwardVector()))
				mainCharacterMovement->ProcessRotateHeavyObject(true, holdComponent->GetHoldingObjectMass(), holdComponent->GetHoldingObjectLocation());
			else if (!FMath::IsNearlyZero(pushingAxis))
			{
				if (pushingAxis == 1.0f)// && holdComponent->CanPushForward(GetActorForwardVector()))
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

void	AMainCharacter::Stick()
{
	if (!holdComponent)
		return;
	holdComponent->Stick();
}

void	AMainCharacter::Jump(FVector direction)
{
	if (bBlocked)
		return;
	if (Climb())
		return;
	mainCharacterMovement->SetJumpDirection(direction);
	ACharacter::Jump();
}

bool	AMainCharacter::Climb()
{
	FVector	normal, hitLocation, topPoint;
	bool firstRes = climbTrace(hitLocation, normal, topPoint);
	if (!firstRes)
		return false;
	
	float angle = FMath::RadiansToDegrees(FMath::Acos(GetActorForwardVector() | (normal * -1.0f)));
	if (angle > ClimbAngleTolerence)
		return false;
	
	FVector	newLoc = hitLocation + normal * 50.0f;
	newLoc.Z = GetActorLocation().Z;
	climbPointTarget = topPoint;

	FLatentActionInfo	latentInfo;
	latentInfo.CallbackTarget = this;
	float interpTime = (newLoc - GetActorLocation()).Size() / 100.0f;
	UKismetSystemLibrary::MoveComponentTo(GetCapsuleComponent(), newLoc, (normal * -1.0f).Rotation(), true, true, interpTime, true, EMoveComponentAction::Type::Move, latentInfo);
	FTimerDelegate	del;
	del.BindUFunction(this, "endCharacterClimbSnap");
	GetWorldTimerManager().SetTimer(climbSnapTimerHandle, del, interpTime, false);
	BlockCharacter();
	return true;
}

void	AMainCharacter::OnDamage()
{
	if (holdComponent)
		holdComponent->UniversalRelease();
	stopCurrentPlayingMontage();
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

void	AMainCharacter::HeadLookAt(FVector lookAtLocation)
{
	if (auto* mesh = GetMesh())
	{
		FVector headLocation = mesh->GetBoneLocation("Maori_Chest_JNT");
		FRotator neededRot = (lookAtLocation - headLocation).Rotation() - GetActorRotation();
		SetHeadRotation(neededRot);
	}
}

void	AMainCharacter::EndClimb()
{
	isClimbing = false;
	//GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	//GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -92.0f));
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
	SetActorLocation(climbPointTarget + FVector::UpVector * (characterCapsule->GetScaledCapsuleHalfHeight() + 10.0f),
		false, nullptr, ETeleportType::TeleportPhysics);
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

void	AMainCharacter::SetHeadRotation(FRotator value)
{
	headRotation.Pitch = FMath::Clamp(value.Pitch, -MaxHeadRotationRange.Pitch, MaxHeadRotationRange.Pitch);
	headRotation.Yaw = FMath::Clamp(value.Yaw, -MaxHeadRotationRange.Yaw, MaxHeadRotationRange.Yaw);
}

void	AMainCharacter::SetCustomSpeed(bool customSpeed, float newSpeed)
{
	bCustomSpeedEnabled = true;
	mainCharacterMovement->MaxWalkSpeed = customSpeed ? newSpeed : mainCharacterMovement->WalkSpeed;
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

bool	AMainCharacter::CanThrow() const
{
	return bHoldingObject || bMovingHeavyObject;
}

bool	AMainCharacter::IsInAir() const
{
	bool ascending = false;
	return mainCharacterMovement->IsFalling(ascending);
}

FVector	AMainCharacter::GetTwoHandsLocation() const
{
	if (auto* mesh = GetMesh())
	{
		FVector	leftHandLocation = mesh->GetBoneLocation("Maori_L_Hand_JNT");
		FVector	rightHandLocation = mesh->GetBoneLocation("Maori_R_Hand_JNT");
		FVector	distance = rightHandLocation - leftHandLocation;
		return leftHandLocation + distance * 0.5f;
	}
	return FVector::ZeroVector;
}

bool	AMainCharacter::climbTrace(FVector& outHitLocation, FVector& outNormal, FVector& outTopPoint)
{
	UCapsuleComponent* capsuleComponent = GetCapsuleComponent();
		FCollisionShape	firstCapsule = FCollisionShape::MakeCapsule(capsuleComponent->GetScaledCapsuleRadius(), 49);
	FCollisionShape	secondCapsule = FCollisionShape::MakeCapsule(capsuleComponent->GetScaledCapsuleRadius(), 99);
	FCollisionQueryParams	queryParams;
	queryParams.AddIgnoredActor(this);

	FVector	beginGround = GetActorLocation();
	FVector	groundEnd = GetActorLocation() + GetActorForwardVector() * 50.0f;
	beginGround.Z -= capsuleComponent->GetScaledCapsuleHalfHeight();
	groundEnd.Z -= capsuleComponent->GetScaledCapsuleHalfHeight();
	
	FVector twoMeterBegin = beginGround;
	twoMeterBegin.Z += 150;
	FVector	twoMeterEnd = groundEnd;
	twoMeterEnd.Z += 150;
	

	FHitResult	twoMeterHitResult;
	if (GetWorld()->SweepSingleByChannel(twoMeterHitResult, twoMeterBegin, twoMeterEnd,
		FQuat::Identity, ECollisionChannel::ECC_GameTraceChannel1, firstCapsule, queryParams))
	{
		FHitResult upperHitResult;
		FVector upperBegin = beginGround;
		upperBegin.Z += 300;
		FVector	upperEnd = groundEnd;
		upperEnd.Z += 300;
		
		if (!GetWorld()->SweepSingleByChannel(upperHitResult, upperBegin, upperEnd, FQuat::Identity, ECollisionChannel::ECC_Visibility, secondCapsule, queryParams))
		{
			twoMeterHitResult.GetComponent()->GetClosestPointOnCollision(upperEnd, outTopPoint);
			outHitLocation = twoMeterHitResult.Location - twoMeterHitResult.Normal * secondCapsule.GetCapsuleRadius();
			outNormal = twoMeterHitResult.Normal;
			return true;
		}
		return false;
	}
	else
	{
		FVector oneMeterBegin = beginGround;
		oneMeterBegin.Z += 50;
		FVector	oneMeterEnd = groundEnd;
		oneMeterEnd.Z += 50;
		FHitResult	oneMeterHitResult;
		
		if (GetWorld()->SweepSingleByChannel(oneMeterHitResult, oneMeterBegin, oneMeterEnd, 
			FQuat::Identity, ECollisionChannel::ECC_GameTraceChannel1, firstCapsule, queryParams))
		{
			FHitResult upperHitResult;
			FVector upperBegin = beginGround;
			upperBegin.Z += 200;
			FVector	upperEnd = groundEnd;
			upperEnd.Z += 200;

			if (!GetWorld()->SweepSingleByChannel(upperHitResult, upperBegin, upperEnd, FQuat::Identity, ECollisionChannel::ECC_Visibility, secondCapsule, queryParams))
			{
				oneMeterHitResult.GetComponent()->GetClosestPointOnCollision(twoMeterEnd, outTopPoint);
				outHitLocation = oneMeterHitResult.Location - oneMeterHitResult.Normal * firstCapsule.GetCapsuleRadius();
				outNormal = oneMeterHitResult.Normal;
				return true;
			}
			return false;
		}
		return false;
	}
}

void	AMainCharacter::endCharacterClimbSnap()
{
	PlayAnimMontage(ClimbMontage);
	beginClimbActorLocation = GetActorLocation();
	//GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	isClimbing = true;
}
	
void	AMainCharacter::updateClimbAnimationTranslation()
{
	UAnimMontage* montage = GetCurrentMontage();
	if (montage == ClimbMontage && GetMesh())
	{
		UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
		if (animInstance)
		{
			float outZValue = 0.0f;
			float outXValue = 0.0f;
			animInstance->GetCurveValue("ZOffset", outZValue);
			animInstance->GetCurveValue("XOffset", outXValue);
			FVector newLoc = beginClimbActorLocation + FVector(outXValue, 0.0f, outZValue);
			SetActorLocation(newLoc);
		}
	}
}
	
void	AMainCharacter::stopCurrentPlayingMontage()
{
	UAnimMontage*	montage = GetCurrentMontage();
	if (montage == ClimbMontage)
	{

	}
	else if (montage == LightGrabAnim)
	{
		if (holdComponent)
			holdComponent->CancelLightGrab();
		StopAnimMontage(montage);
	}
	else if (montage == LightThrowAnim)
	{
		if (holdComponent)
			holdComponent->CancelThrow();
		StopAnimMontage(montage);
	}
	else if (montage == HeavyGrabAnim)
	{
		if (holdComponent)
			holdComponent->CancelHeavyGrab();
		StopAnimMontage(montage);
	}
	else if (montage == HeavyPushAnim)
	{
		if (holdComponent)
			holdComponent->CancelHeavyThrow();
		StopAnimMontage(montage);
	}
}