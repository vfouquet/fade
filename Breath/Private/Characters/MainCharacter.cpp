// Fill out your copyright notice in the Description page of Project Settings.

#include "MainCharacter.h"

#include "HoldComponent.h"
#include "MainCharacterMovementComponent.h"

#include "GameFramework/SpringArmComponent.h"
#include "Cameras/PlayerCameraComponent.h"
#include "MainPlayerController.h"
#include "Camera/CameraActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AkAudio/Classes/AkGameplayStatics.h"
#include "IdentityEraserComponent.h"
#include "MemoryZoneComponent.h"
#include "ChemicalFireComponent.h"

// Sets default values
AMainCharacter::AMainCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UMainCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComponent->SetupAttachment(this->GetRootComponent());
	mainCharacterMovement = Cast<UMainCharacterMovementComponent>(GetCharacterMovement());

	UCapsuleComponent*	capsule = GetCapsuleComponent();
	FScriptDelegate	beginOverlapDel;
	beginOverlapDel.BindUFunction(this, "OnCapsuleOverlap");
	capsule->OnComponentBeginOverlap.Add(beginOverlapDel);
	FScriptDelegate	endOverlapDel;
	endOverlapDel.BindUFunction(this, "OnCapsuleEndOverlap");
	capsule->OnComponentEndOverlap.Add(endOverlapDel);
	FScriptDelegate	hitOverlap;
	hitOverlap.BindUFunction(this, "OnCapsuleHit");
	capsule->OnComponentHit.Add(hitOverlap);
}

// Called when the game starts or when spawned
void AMainCharacter::BeginPlay()
{
	Super::BeginPlay();
	holdComponent = FindComponentByClass<UHoldComponent>();
	

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

	updateHealthValues(DeltaTime);

	bool ascending = false;
	if (mainCharacterMovement->IsFalling(ascending) && !isClimbing && !bBlocked)
	{
		if (bCanAutoClimbInAir)
			Climb();
	}

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
				mainCharacterMovement->ProcessRotateHeavyObject(false, holdComponent->GetCurrentHeldObject(), holdComponent->GetHoldingObjectLocation());
			else if (rotatingRight)// && holdComponent->CanRotateRight(GetActorForwardVector()))
				mainCharacterMovement->ProcessRotateHeavyObject(true, holdComponent->GetCurrentHeldObject(), holdComponent->GetHoldingObjectLocation());
			else if (!FMath::IsNearlyZero(pushingAxis))
			{
				if (pushingAxis == 1.0f)// && holdComponent->CanPushForward(GetActorForwardVector()))
					mainCharacterMovement->ProcessPushAndPull(pushingAxis, holdComponent->GetCurrentHeldObject());
				else if (pushingAxis == -1.0f)
					mainCharacterMovement->ProcessPushAndPull(pushingAxis, holdComponent->GetCurrentHeldObject());
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
	{
		GetCharacterMovement()->AddInputVector(Value);
		//GetMesh()->SetWorldRotation(Value.Rotation().Quaternion());
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
	if (bBlocked || bJumpLocked || bMovingHeavyObject)
		return;
	if (Climb())
		return;
	bIsLongJump = GetVelocity().Size() >= MinLongJumpVelocity;
	mainCharacterMovement->SetJumpDirection(FVector::ZeroVector);
	Super::Jump();
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

	FVector	newLoc;
	if (climbType == EClimbType::OneMeterClimb)
		newLoc = hitLocation + normal * 65.0f;
	else if (climbType == EClimbType::AirOneMeterClimb)
		newLoc = hitLocation + normal * 45.0f;
	else
		newLoc = hitLocation + normal * 50.0f;

	if (climbType == EClimbType::OneMeterClimb)
		newLoc.Z = GetActorLocation().Z;
	else if (climbType == EClimbType::TwoMetersClimb)
		newLoc.Z = GetActorLocation().Z;
	else if (climbType == EClimbType::AirOneMeterClimb)
	{
		float rootLocZ = GetMesh()->GetBoneLocation("Maori_Root_JNT").Z;
		newLoc.Z = topPoint.Z - 100.0f + 92.0f;
	}
	else if (climbType == EClimbType::AirTwoMetersClimb)
	{
		float rootLocZ = GetMesh()->GetBoneLocation("Maori_Root_JNT").Z;
		newLoc.Z = topPoint.Z - 200.0f + 92.0f;
	}

	FRotator newRotation = FRotator::ZeroRotator;
	newRotation.Yaw = (-1.0f * normal).Rotation().Yaw;
	FTimerDelegate	del;
	del.BindUFunction(this, "endCharacterClimbSnap");
	SnapCharacterTo(newLoc, newRotation, ClimbSnapTime, del);

	return true;
}

void	AMainCharacter::SnapCharacterTo(FVector location, FRotator rotation, float time, FTimerDelegate& del)
{
	FLatentActionInfo	latentInfo;
	latentInfo.CallbackTarget = this;
	UKismetSystemLibrary::MoveComponentTo(GetCapsuleComponent(), location, rotation, true, true, time, true, EMoveComponentAction::Type::Move, latentInfo);
	GetWorldTimerManager().SetTimer(climbSnapTimerHandle, del, time, false);
	BlockCharacter();
}
	
void	AMainCharacter::StopSnapping()
{
	FLatentActionInfo	latentInfo;
	latentInfo.CallbackTarget = this;
	UKismetSystemLibrary::MoveComponentTo(GetCapsuleComponent(), FVector::ZeroVector, FRotator::ZeroRotator, true, true, 0.0f, true, EMoveComponentAction::Type::Stop, latentInfo);
	GetWorldTimerManager().ClearTimer(climbSnapTimerHandle);
}

void	AMainCharacter::EndThrow()
{
	if (holdComponent)
		holdComponent->EndThrow();
}

void	AMainCharacter::BeginGrabPositionUpdate()
{
	if (holdComponent)
		holdComponent->BeginLightGrabPositionUpdate();
}
	
void	AMainCharacter::EndReleaseLightGrab()
{
	if (holdComponent)
		holdComponent->EndLightGrabRelease();
}

void	AMainCharacter::DealDamage(FHitResult hitResult, UPrimitiveComponent* damageDealer, bool HeavyDamage)
{
	if (bIsGod || bIsDead)
		return;

	float* potentialValue = damageDealers.Find(damageDealer);
	if (potentialValue)
		return;

	if (!HeavyDamage)
	{
		if (currentDamageState == ECharacterDamageState::Wounded)
			Die(hitResult.ImpactNormal * ImpactMeshForce, hitResult.Location, hitResult.BoneName);
		else
		{
			damageDealers.Add(damageDealer, 0.0f);
			OnDamage(hitResult.ImpactNormal);
			currentDamageState = ECharacterDamageState::Wounded;
			onDamageStateChanged.Broadcast(ECharacterDamageState::None, ECharacterDamageState::Wounded);
		}
	}
	else
		Die(hitResult.ImpactNormal * ImpactMeshForce, hitResult.Location, hitResult.BoneName);
}

void	AMainCharacter::OnDamage(FVector impactDir)
{
	if (holdComponent)
		holdComponent->UniversalRelease();
	stopCurrentPlayingMontage();
	onCharacterDamage.Broadcast(impactDir);
}

void	AMainCharacter::Die(FVector impact, FVector impactLocation, FName boneName)
{
	bIsDead = true;
	OnDie.Broadcast();
	BlockCharacter();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	USkeletalMeshComponent* mesh = GetMesh();
	if (!mesh)
		return;
	mesh->SetCollisionProfileName("CharacterMeshRagdoll");
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
	
void	AMainCharacter::SetWalkingInDeepWater(bool const value)
{
	bIsWalkingInDeepWater = value; 
	UAkGameplayStatics::SetSwitch("Footstep", "Water_high", this);
}

void	AMainCharacter::SetWalkMode()
{
	bool bAscending = false;

	if (!bCustomSpeedEnabled && this->mainCharacterMovement->IsFalling(bAscending) == false)
	mainCharacterMovement->SetWalkMode();
}

void	AMainCharacter::SetJogMode()
{
	bool bAscending = false;

	if (!bCustomSpeedEnabled && this->mainCharacterMovement->IsFalling(bAscending) == false)
		mainCharacterMovement->SetJogMode();
}

void	AMainCharacter::SetHeadRotation(FRotator value)
{
	headRotation.Pitch = FMath::Clamp(value.Pitch, -MaxHeadRotationRange.Pitch, MaxHeadRotationRange.Pitch);
	headRotation.Yaw = FMath::Clamp(value.Yaw, -MaxHeadRotationRange.Yaw, MaxHeadRotationRange.Yaw);
}

void	AMainCharacter::SetCustomSpeed(bool customSpeed, float newSpeed)
{
	bCustomSpeedEnabled = true;
	mainCharacterMovement->SetCustomSpeed(customSpeed ? newSpeed : mainCharacterMovement->WalkSpeed);
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
	
void	AMainCharacter::PlayLightGrabReleaseMontage()
{
	PlayAnimMontage(LightGrabReleaseAnim);
	FOnMontageEnded	endDel;
	endDel.BindUObject(this, &AMainCharacter::onEndMontage);
	GetMesh()->GetAnimInstance()->Montage_SetEndDelegate(endDel, LightGrabReleaseAnim);
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
	bMovingHeavyObject = true;
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

void	AMainCharacter::PlayMeteorEvent()
{
	UAkGameplayStatics::PostEvent(MeteorEvent, this);
}

void	AMainCharacter::StopMeteorEvent()
{
	UAkGameplayStatics::PostEvent(StoppingMeteorEvent, this);
}

bool	AMainCharacter::CanThrow() const
{
	return bHoldingObject;// || bMovingHeavyObject;
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
	
AActor*	AMainCharacter::GetHeldActor()
{
	if (holdComponent)
	{
		auto* heldObject = holdComponent->GetCurrentHeldObject();
		return heldObject ? heldObject->GetOwner() : nullptr;
	}
	return nullptr;
}
	
FVector		AMainCharacter::GetHoldSocketLocation()
{
	if (auto* mesh = GetMesh())
	{
		return mesh->GetSocketLocation("HoldSocket");
	}
	UE_LOG(LogTemp, Error, TEXT("Main Character : Can't get HoldSocket location because mesh is nullptr"));
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
	else if (montage == LightGrabReleaseAnim)
	{
		if (bInterrupted && holdComponent && holdComponent->GetCurrentState() == EHoldingState::ReleasingLightGrab)
			holdComponent->EndLightGrabRelease();
	}
	else if (montage == JumpReceptionAnim)
		bBlocked = false;
}

void	AMainCharacter::endCharacterClimbSnap()
{
	beginClimbActorLocation = GetActorLocation();
	
	if (auto* mesh = GetMesh())
	{
		hipBeginLocation = mesh->GetBoneLocation("Maori_Hip_JNT");
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		
		FOnMontageEnded	endDel;
		endDel.BindUObject(this, &AMainCharacter::onEndMontage);
		if (climbType == EClimbType::OneMeterClimb)
		{
			PlayAnimMontage(Climb1MeterMontage);
			mesh->GetAnimInstance()->Montage_SetEndDelegate(endDel, Climb1MeterMontage);
		}
		else if (climbType == EClimbType::TwoMetersClimb || climbType == EClimbType::AirTwoMetersClimb)
		{
			PlayAnimMontage(Climb2MetersMontage);
			mesh->GetAnimInstance()->Montage_SetEndDelegate(endDel, Climb2MetersMontage);
		}
		else if (climbType == EClimbType::AirOneMeterClimb)
		{
			PlayAnimMontage(Climb1MeterMontage, 1.0f, "InAir");
			mesh->GetAnimInstance()->Montage_SetEndDelegate(endDel, Climb1MeterMontage);
		}
		isClimbing = true;
		mainCharacterMovement->SetMovementMode(EMovementMode::MOVE_Flying);	
	}
	mainCharacterMovement->EndJumping();
}
	
void	AMainCharacter::endClimb()
{
	isClimbing = false;
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -92.0f));
	mainCharacterMovement->SetMovementMode(EMovementMode::MOVE_Walking);
	UnblockCharacter();
	mainCharacterMovement->bOrientRotationToMovement = true;
	climbType = EClimbType::None;
}
	
void	AMainCharacter::updateHealthValues(float DeltaTime)
{
	for (auto& damageDealer : damageDealers)
	{
		damageDealer.Value += DeltaTime;
		if (damageDealer.Value >= 0.5f)
			damageDealers.Remove(damageDealer.Key);
	}

	if (bIsGod)
		return;

	if (bIsDead)
		return;

	bool ascending = false;
	if (mainCharacterMovement && mainCharacterMovement->IsFalling(ascending) && !ascending)
	{
		jumpZOffset -= mainCharacterMovement->GetLastMovementOffset().Z;
		if (jumpZOffset >= FatalJumpHeight)
			Die();
	}
	else
	{
		if (jumpZOffset > ReceptionJumpHeight)
		{
			PlayAnimMontage(JumpReceptionAnim);
			FOnMontageEnded	endDel;
			endDel.BindUObject(this, &AMainCharacter::onEndMontage);
			GetMesh()->GetAnimInstance()->Montage_SetEndDelegate(endDel, JumpReceptionAnim);
			bBlocked = true;
		}
		jumpZOffset = 0.0f;
	}

	if (currentCondition != ECharacterCondition::None && isAffectedByFire())
	{
		currentFireTime += DeltaTime;
		if (currentFireTime >= (currentCondition == ECharacterCondition::Scalding ? ScaldingToBurning : BurningToDeath))
		{
			if (currentCondition == ECharacterCondition::Scalding)
			{
				currentCondition = ECharacterCondition::Burning;
				onConditionChanged.Broadcast(ECharacterCondition::Scalding, ECharacterCondition::Burning);
				OnDamage();
			}
			else if (currentCondition == ECharacterCondition::Burning)
				Die();
			currentFireTime = 0.0f;
		}
	}
	if (currentDamageState == ECharacterDamageState::Wounded)
	{
		currentDamageTime += DeltaTime;
		if (currentDamageTime >= WoundedResetTime)
		{
			currentDamageTime = 0.0f;
			currentDamageState = ECharacterDamageState::None;
			onDamageStateChanged.Broadcast(ECharacterDamageState::Wounded, ECharacterDamageState::None);
		}
	}
}

void	AMainCharacter::decreaseFireCount()
{
	if (fireCount == 1)
	{
		fireCount = 0;
		if (currentCondition == ECharacterCondition::Scalding)
		{
			onConditionChanged.Broadcast(ECharacterCondition::Scalding, ECharacterCondition::None);
			currentCondition = ECharacterCondition::None;
			currentFireTime = 0.0f;
		}
	}
	else if (fireCount > 1)
		fireCount--;
}

void	AMainCharacter::applyWaterEffect()
{
	if (currentCondition == ECharacterCondition::Scalding || currentCondition == ECharacterCondition::Burning)
	{
		onConditionChanged.Broadcast(currentCondition, ECharacterCondition::None);
		currentCondition = ECharacterCondition::None;
		currentFireTime = 0.0f;
	}
}

void	AMainCharacter::OnCapsuleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (bIsDead)
		return;

	UIdentityEraserComponent* identityEraser = Cast<UIdentityEraserComponent>(OtherComp);
	if (identityEraser)
	{
		eraseZoneCount++;
		if (memoryZoneCount == 0)
		{
			currentFireTime = 0.0f;
			if (eraseZoneCount == 1)
				onCharacterErased.Broadcast(identityEraser);
		}
	}

	UMemoryZoneComponent* memoryZoneEraser = Cast<UMemoryZoneComponent>(OtherComp);
	if (memoryZoneEraser)
	{
		if (memoryZoneCount == 0 && eraseZoneCount != 0)
			onCharacterGetIdentity.Broadcast(nullptr);
		memoryZoneCount++;
	}

	UChemicalComponent*	comp = UChemicalComponent::FindAssociatedChemicalComponent(OtherComp);
	if (!comp)
		return;
	if ((comp->GetType() == EChemicalType::Fire && comp->GetState() == EChemicalState::None) ||
		((comp->GetType() == EChemicalType::Rock || comp->GetType() == EChemicalType::Wood) && comp->GetState() == EChemicalState::Burning))
	{
		fireCount++;
		if (bIsGod)
			return;
		UChemicalFireComponent* fireComp = Cast<UChemicalFireComponent>(comp);
		if (currentCondition != ECharacterCondition::Burning && fireComp && fireComp->bInstantBurningPlayer)
		{
			onConditionChanged.Broadcast(currentCondition, ECharacterCondition::Burning);
			currentCondition = ECharacterCondition::Burning;
			OnDamage();
		}
		else if (currentCondition == ECharacterCondition::None)
		{
			onConditionChanged.Broadcast(ECharacterCondition::None, ECharacterCondition::Scalding);
			currentCondition = ECharacterCondition::Scalding;
			OnDamage();
		}
	}
	else if (comp->GetType() == EChemicalType::Water)
		applyWaterEffect();
}

void	AMainCharacter::OnCapsuleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UIdentityEraserComponent* identityEraser = Cast<UIdentityEraserComponent>(OtherComp);
	if (identityEraser)
	{
		if (memoryZoneCount == 0 && eraseZoneCount == 1)
		{
			onCharacterGetIdentity.Broadcast(nullptr);
		}
		eraseZoneCount--;
	}

	UMemoryZoneComponent* memoryZoneEraser = Cast<UMemoryZoneComponent>(OtherComp);
	if (memoryZoneEraser)
	{
		if (memoryZoneCount == 1 && eraseZoneCount > 0)
			onCharacterErased.Broadcast(nullptr);
		memoryZoneCount--;
	}

	UChemicalComponent*	comp = UChemicalComponent::FindAssociatedChemicalComponent(OtherComp);
	if (!comp)
		return;
	if ((comp->GetType() == EChemicalType::Fire && comp->GetState() == EChemicalState::None) ||
		((comp->GetType() == EChemicalType::Rock || comp->GetType() == EChemicalType::Wood) && comp->GetState() == EChemicalState::Burning))
		decreaseFireCount();
}