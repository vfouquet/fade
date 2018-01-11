// Fill out your copyright notice in the Description page of Project Settings.

#include "MoveComponent.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "BoxClimbComponent.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"

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
	for (auto&& climbBoxesReference : climbBoxesReferences)
	{
		UBoxClimbComponent*	tempBox = Cast<UBoxClimbComponent>(climbBoxesReference.GetComponent(GetOwner()));
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
void UMoveComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (isBlocked)
	{
		if (isMovingHeavyObject)
			return;
		if (currentInputValue == FVector2D::ZeroVector)
			return;

		ACharacter* Char = Cast<ACharacter>(GetOwner());

		float	inputAngle = GetInputAngle();
		APlayerController* CharacterController = Cast<APlayerController>(Char->GetController());
		FRotator CamRot = CharacterController->PlayerCameraManager->GetCameraRotation();
		FRotator playerForward = (holdingObjectLocation - Char->GetActorLocation()).Rotation();
		float cameraDiffAngle = CamRot.Yaw - Char->GetActorRotation().Yaw;


		inputAngle += 90.0f;
		if (inputAngle < 0.0f)
			inputAngle += 360.0f;

		if (cameraDiffAngle < 0.0f)
			cameraDiffAngle += 360.0f;

		float difference = inputAngle - cameraDiffAngle;

		if (difference < 0.0f)
			difference += 360.0f;

		FRotator characterRot = Char->GetActorRotation();
		characterRot.Yaw += ThrowRotationSpeed * DeltaTime * (difference >= 0.0f && difference < 180.0f) ? 1.0f : -1.0f;
		Char->SetActorRotation(characterRot);
		return;
	}

	if (canClimb && !isHoldingObject && !isMovingHeavyObject)
	{
		ACharacter* Char = Cast<ACharacter>(GetOwner());
		FVector vel = Char->GetCharacterMovement()->Velocity;
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

	if (isMovingHeavyObject)
	{
		if (currentInputValue == FVector2D::ZeroVector)
			return;

		ACharacter* Char = Cast<ACharacter>(GetOwner());
		
		float	inputAngle = GetInputAngle();
		APlayerController* CharacterController = Cast<APlayerController>(Char->GetController());

		FRotator CamRot = CharacterController->PlayerCameraManager->GetCameraRotation();
		FRotator playerForward = (holdingObjectLocation - Char->GetActorLocation()).Rotation();
		float cameraDiffAngle = CamRot.Yaw - Char->GetActorRotation().Yaw;
	

		inputAngle += 90.0f;
		if (inputAngle < 0.0f)
			inputAngle += 360.0f;
	
		if (cameraDiffAngle < 0.0f)
			cameraDiffAngle += 360.0f;

		float difference = inputAngle - cameraDiffAngle;

		if (difference < 0.0f)
			difference += 360.0f;

		if (difference >= 180.0f -HeavyAngleTolerance && difference <= 180.0f + HeavyAngleTolerance)
		{
			float	massMult = 0.0f;
			if (holdingObjectMass <= MassGrabValues[0])
				massMult = MassGrabMultipliers[0];
			else if (holdingObjectMass <= MassGrabValues[1])
				massMult = MassGrabMultipliers[1];
			else if (holdingObjectMass <= MassGrabValues[2])
				massMult = MassGrabMultipliers[2];

			FVector MoveDir = Char->GetActorRotation().Vector();
			Char->GetCharacterMovement()->AddInputVector(MoveDir * massMult);
		}
		else if (difference >= 360.0f - HeavyAngleTolerance || difference <= HeavyAngleTolerance)
		{
			float	massMult = 0.0f;
			if (holdingObjectMass <= MassGrabValues[0])
				massMult = MassGrabMultipliers[0];
			else if (holdingObjectMass <= MassGrabValues[1])
				massMult = MassGrabMultipliers[1];
			else if (holdingObjectMass <= MassGrabValues[2])
				massMult = MassGrabMultipliers[2];

			FVector MoveDir = Char->GetActorRotation().Vector();
			Char->GetCharacterMovement()->AddInputVector(MoveDir * -1.0f * massMult);
		}
		else
		{
			float	massMult = 0.0f;
			if (holdingObjectMass <= MassGrabValues[0])
				massMult = MassGrabMultipliers[0];
			else if (holdingObjectMass <= MassGrabValues[1])
				massMult = MassGrabMultipliers[1];
			else if (holdingObjectMass <= MassGrabValues[2])
				massMult = MassGrabMultipliers[2];
			float angle = RotationSpeed * GetWorld()->GetDeltaSeconds() * massMult;	
			angle *= (difference >= 0.0f && difference < 180.0f) ? -1.0f : 1.0f;
			//MAYBE USE OBJECT WEIGHT
			FRotator	rotation = FQuat(FVector::UpVector, FMath::DegreesToRadians(angle)).Rotator();
			FVector		holdingToCharac = Char->GetActorLocation() - holdingObjectLocation;
			FVector		holdingToNewLoc = rotation.RotateVector(holdingToCharac);
			FVector		finalLocation = holdingToNewLoc + holdingObjectLocation;
			FRotator	finalRotation = rotation + Char->GetActorRotation();
			Char->SetActorLocationAndRotation(finalLocation, finalRotation.Quaternion(), true, nullptr, ETeleportType::None);
		}
	}
	else
	{
		float stickLength = currentInputValue.Size();
		if (stickLength < JogStickThreshold)
			setWalkMode();
		else
			setJogMode();
	}
}

void UMoveComponent::MoveForward(float Value)
{
	currentInputValue.Y = Value;
	if (isBlocked)
		return;

	ACharacter* Char = Cast<ACharacter>(GetOwner());
	APlayerController* CharacterController = Cast<APlayerController>(Char->GetController());

	if (!isMovingHeavyObject)
	{
		if (Value <= WalkStickThreshold && Value >= -WalkStickThreshold)
			return;

		FRotator CamRot = CharacterController->PlayerCameraManager->GetCameraRotation();
		CamRot.Pitch = 0.0f;
		FVector MoveDir = CamRot.Vector();
		if (Value < 0.0f)
			Char->GetCharacterMovement()->AddInputVector(MoveDir * -1.0f);
		else
			Char->GetCharacterMovement()->AddInputVector(MoveDir);
	}
}

void UMoveComponent::MoveRight(float Value)
{
	currentInputValue.X = Value;
	if (isBlocked)
		return;

	ACharacter* Char = Cast<ACharacter>(GetOwner());
	APlayerController* CharacterController = Cast<APlayerController>(Char->GetController());

	if (!isMovingHeavyObject)
	{
		if (Value <= WalkStickThreshold && Value >= -WalkStickThreshold)
			return;

		FRotator CamRot = CharacterController->PlayerCameraManager->GetCameraRotation();
		CamRot.Pitch = 0.0f;
		FVector MoveDir = CamRot.RotateVector(FVector::RightVector);
		if (Value < 0.0f)
			Char->GetCharacterMovement()->AddInputVector(MoveDir * -1.0f);
		else
			Char->GetCharacterMovement()->AddInputVector(MoveDir);;
	}
}

bool	UMoveComponent::Climb()
{
	computeClimbableBoxes();
	if (!canClimb)
		return false;

	//BEGIN SNAP + BLOCK INPUT
	ACharacter* character = Cast<ACharacter>(GetOwner());
	if (!character)
		return false;
	UCapsuleComponent*	characterCapsule = character->FindComponentByClass<UCapsuleComponent>();
	if (!characterCapsule)
		return false;
	character->SetActorLocation(validClimbableBox->GetClimbedLocation() + FVector::UpVector * (characterCapsule->GetScaledCapsuleHalfHeight() + 10.0f), 
			false, nullptr, ETeleportType::TeleportPhysics);
	return true;
}

float	UMoveComponent::GetCameraTargetDiffAngle() const
{
	ACharacter* Char = Cast<ACharacter>(GetOwner());
	APlayerController* CharacterController = Cast<APlayerController>(Char->GetController());

	FRotator CamRot = CharacterController->PlayerCameraManager->GetCameraRotation();
	return CamRot.Yaw - Char->GetActorRotation().Yaw;
}

bool	UMoveComponent::IsInAir() const
{
	ACharacter* Char = Cast<ACharacter>(GetOwner());
	if (!Char)
		return false;
	return Char->GetCharacterMovement()->IsFalling();
}

void	UMoveComponent::EnableMovingHeavyObjectMode()
{
	isMovingHeavyObject = true;
	ACharacter* Char = Cast<ACharacter>(GetOwner());
	if (Char)
		Char->GetCharacterMovement()->bOrientRotationToMovement = false;
}

void	UMoveComponent::DisableMovingHeavyObjectMode()
{
	isMovingHeavyObject = false;
	ACharacter* Char = Cast<ACharacter>(GetOwner());
	if (Char)
		Char->GetCharacterMovement()->bOrientRotationToMovement = true;
}

void	UMoveComponent::setWalkMode()
{
	ACharacter* Char = Cast<ACharacter>(GetOwner());
	if (Char)
	{
		Char->GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}
}

void	UMoveComponent::setJogMode()
{
	ACharacter* Char = Cast<ACharacter>(GetOwner());
	if (Char)
	{
		Char->GetCharacterMovement()->MaxWalkSpeed = JogSpeed;
	}
}
	
void	UMoveComponent::computeClimbableBoxes()
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

bool	UMoveComponent::isClimbAngleCorrect() const
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