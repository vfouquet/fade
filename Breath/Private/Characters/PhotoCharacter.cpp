// Fill out your copyright notice in the Description page of Project Settings.

#include "PhotoCharacter.h"
#include "Components/SkeletalMeshComponent.h"

// Sets default values
APhotoCharacter::APhotoCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bTickEvenWhenPaused = true;
}

// Called when the game starts or when spawned
void APhotoCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (auto* mesh = GetMesh())
		mesh->SetTickableWhenPaused(true);
	photoCamera = FindComponentByClass<UCameraComponent>();
}

// Called every frame
void APhotoCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void APhotoCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void	APhotoCharacter::MoveForward(float value)
{
}

void	APhotoCharacter::MoveRight(float value)
{
}

void	APhotoCharacter::RotateHorizontal(float value)
{
	AddControllerYawInput(value * RotateSpeed * GetWorld()->GetDeltaSeconds());
}

void	APhotoCharacter::RotateVertical(float value)
{
	AddControllerPitchInput(value * RotateSpeed * GetWorld()->GetDeltaSeconds());
}

void	APhotoCharacter::PlayPreviousBodyPose()
{
	if (BodyPoses.Num() <= 1)
		return;

	currentBodyIndex = currentBodyIndex < BodyPoses.Num() - 1 ? currentBodyIndex + 1 : 0;
	StopAnimMontage(lastPlayingBodyMontage);
	PlayAnimMontage(BodyPoses[currentBodyIndex].AssociatedMontage);
	lastPlayingBodyMontage = BodyPoses[currentBodyIndex].AssociatedMontage;
}

void	APhotoCharacter::PlayNextBodyPose()
{
	if (BodyPoses.Num() <= 1)
		return;

	currentBodyIndex = currentBodyIndex > 0 ? currentBodyIndex - 1 : BodyPoses.Num() - 1;
	StopAnimMontage(lastPlayingBodyMontage);
	PlayAnimMontage(BodyPoses[currentBodyIndex].AssociatedMontage);
	lastPlayingBodyMontage = BodyPoses[currentBodyIndex].AssociatedMontage;
}

void	APhotoCharacter::PlayPreviousFacePose()
{
	if (FacePoses.Num() <= 1)
		return;

	currentFaceIndex = currentFaceIndex < FacePoses.Num() - 1 ? currentFaceIndex + 1 : 0;
	StopAnimMontage(lastPlayingFaceMontage);
	PlayAnimMontage(FacePoses[currentFaceIndex].AssociatedMontage);
	lastPlayingFaceMontage = FacePoses[currentFaceIndex].AssociatedMontage;
}

void	APhotoCharacter::PlayNextFacePose()
{
	if (FacePoses.Num() <= 1)
		return;

	currentFaceIndex = currentFaceIndex > 0 ? currentFaceIndex - 1 : FacePoses.Num() - 1;
	StopAnimMontage(lastPlayingFaceMontage);
	PlayAnimMontage(FacePoses[currentFaceIndex].AssociatedMontage);
	lastPlayingFaceMontage = FacePoses[currentFaceIndex].AssociatedMontage;
}

void	APhotoCharacter::IncreaseFOV()
{
	if (!photoCamera.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("APhotoCharacter : Couldn't increase FOV because the cam is nullptr"));
		return;
	}

	photoCamera->FieldOfView += 1.0f;
}

void	APhotoCharacter::DecreaseFOV()
{
	if (!photoCamera.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("APhotoCharacter : Couldn't decrease FOV because the cam is nullptr"));
		return;
	}

	photoCamera->FieldOfView -= 1.0f;
}

void	APhotoCharacter::IncreaseCameraRoll()
{
	if (!photoCamera.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("APhotoCharacter : Couldn't increase Roll because the cam is nullptr"));
		return;
	}

	FRotator rot = photoCamera->GetComponentRotation();
	rot.Roll += 1.0f;
	photoCamera->SetWorldRotation(rot);
}

void	APhotoCharacter::DecreaseCameraRoll()
{
	if (!photoCamera.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("APhotoCharacter : Couldn't decrease Roll because the cam is nullptr"));
		return;
	}

	FRotator rot = photoCamera->GetComponentRotation();
	rot.Roll -= 1.0f;
	photoCamera->SetWorldRotation(rot);
}