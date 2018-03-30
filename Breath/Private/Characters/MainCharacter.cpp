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
	if (isClimbing)
	{
		FVector translation = GetMesh()->GetBoneLocation("Maori_Hip_JNT") - hipBeginLocation;
		FVector newLoc = beginClimbActorLocation + translation;
		SetActorLocation(newLoc);
		GetMesh()->SetWorldLocation(newLoc - FVector(0.0f, 0.0f, 92.0f) - translation);
	}

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
	climbType = climbTrace(hitLocation, normal, topPoint);
	if (climbType == EClimbType::None)
		return false;

	float angle = FMath::RadiansToDegrees(FMath::Acos(GetActorForwardVector() | (normal * -1.0f)));
	if (angle > ClimbAngleTolerence)
		return false;

	float	interpTime = 0.0f;
	FVector	newLoc;
	newLoc = hitLocation + normal * (climbType == EClimbType::TwoMetersClimb? 50.0f : 65.0f);
	if (climbType == EClimbType::OneMeterClimb)
	{
		newLoc.Z = GetActorLocation().Z;
		interpTime = (newLoc - GetActorLocation()).Size() / 100.0f;
	}
	else if (climbType == EClimbType::TwoMetersClimb)
	{
		newLoc.Z = GetActorLocation().Z;
		interpTime = (newLoc - GetActorLocation()).Size() / 100.0f;
	}
	else if (climbType == EClimbType::AirOneMeterClimb)
	{
		float rootLocZ = GetMesh()->GetBoneLocation("Maori_Root_JNT").Z;
		newLoc.Z = topPoint.Z - 100.0f + 92.0f;
		interpTime = 0.1f;
	}
	else if (climbType == EClimbType::AirTwoMetersClimb)
	{
		float rootLocZ = GetMesh()->GetBoneLocation("Maori_Root_JNT").Z;
		newLoc.Z = topPoint.Z - 200.0f + 92.0f;
		interpTime = 0.1f;
	}
	zClimbTarget = topPoint.Z;

	FLatentActionInfo	latentInfo;
	latentInfo.CallbackTarget = this;
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

void	AMainCharacter::PlayLightGrabMontage(bool oneMeter)
{ 
	PlayAnimMontage(oneMeter ? LightGrabOneMeterAnim : LightGrabAnim);
	FOnMontageEnded	endDel;
	endDel.BindUObject(this, &AMainCharacter::onEndMontage);
	GetMesh()->GetAnimInstance()->Montage_SetEndDelegate(endDel, oneMeter ? LightGrabOneMeterAnim : LightGrabAnim);
}

void	AMainCharacter::PlayLightThrowMontage()
{ 
	PlayAnimMontage(LightThrowAnim); 
	FOnMontageEnded	endDel;
	endDel.BindUObject(this, &AMainCharacter::onEndMontage);
	GetMesh()->GetAnimInstance()->Montage_SetEndDelegate(endDel, LightThrowAnim);
}

void	AMainCharacter::PlayHeavyGrabMontage()
{ 
	PlayAnimMontage(HeavyGrabAnim); 
	FOnMontageEnded	endDel;
	endDel.BindUObject(this, &AMainCharacter::onEndMontage);
	GetMesh()->GetAnimInstance()->Montage_SetEndDelegate(endDel, HeavyGrabAnim);
}

void	AMainCharacter::PlayHeavyThrowMontage()
{
	PlayAnimMontage(HeavyPushAnim); 
	FOnMontageEnded	endDel;
	endDel.BindUObject(this, &AMainCharacter::onEndMontage);
	GetMesh()->GetAnimInstance()->Montage_SetEndDelegate(endDel, HeavyPushAnim);
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

EClimbType	AMainCharacter::climbTrace(FVector& outHitLocation, FVector& outNormal, FVector& outTopPoint)
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

			bool ascending = false;
			return mainCharacterMovement->IsFalling(ascending)? EClimbType::AirTwoMetersClimb : EClimbType::TwoMetersClimb;
		}
		return EClimbType::None;
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
				
				bool ascending = false;
				return mainCharacterMovement->IsFalling(ascending) ? EClimbType::AirOneMeterClimb : EClimbType::OneMeterClimb;
			}
			return EClimbType::None;
		}
		return EClimbType::None;
	}
}

void	AMainCharacter::onEndMontage(UAnimMontage* montage, bool bInterrupted)
{
	if (montage == Climb1MeterMontage || montage == Climb2MetersMontage)
	{
		endClimb();
		return;
	}
	else if (montage == LightGrabAnim)
	{
		if (bInterrupted && holdComponent && !bThrowingObject)
			holdComponent->CancelLightGrab();
		else if (holdComponent)
				holdComponent->EndLightGrab();
	}
	else if (montage == LightThrowAnim)
	{
		if (bInterrupted && holdComponent)
			holdComponent->CancelThrow();
		else if (holdComponent)
			holdComponent->EndThrow();
	}
	else if (montage == HeavyGrabAnim)
	{
		if (bInterrupted && holdComponent)
			holdComponent->CancelHeavyGrab();
		else if (holdComponent)
			holdComponent->EndHeavyGrab();
	}
	else if (montage == HeavyPushAnim)
	{
		if (bInterrupted && holdComponent)
			holdComponent->CancelHeavyThrow();
		else if (holdComponent)
			holdComponent->EndThrow();
	}
}

void	AMainCharacter::endCharacterClimbSnap()
{
	beginClimbActorLocation = GetActorLocation();
	
	if (auto* mesh = GetMesh())
	{
		hipBeginLocation = mesh->GetBoneLocation("Maori_Hip_JNT");
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		if (climbType == EClimbType::OneMeterClimb)
			PlayAnimMontage(Climb1MeterMontage);
		else if (climbType == EClimbType::TwoMetersClimb || climbType == EClimbType::AirTwoMetersClimb)
			PlayAnimMontage(Climb2MetersMontage);
		else if (climbType == EClimbType::AirOneMeterClimb)
			PlayAnimMontage(Climb1MeterMontage, 1.0f, "InAir");
		isClimbing = true;
		mainCharacterMovement->SetMovementMode(EMovementMode::MOVE_Flying);

		FOnMontageEnded	endDel;
		endDel.BindUObject(this, &AMainCharacter::onEndMontage);
		mesh->GetAnimInstance()->Montage_SetEndDelegate(endDel, climbType == EClimbType::TwoMetersClimb ? Climb2MetersMontage : Climb1MeterMontage);
	}
}
	
void	AMainCharacter::endClimb()
{
	isClimbing = false;
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -92.0f));
	mainCharacterMovement->SetMovementMode(EMovementMode::MOVE_Walking);
	UnblockCharacter();
	mainCharacterMovement->bOrientRotationToMovement = true;
}