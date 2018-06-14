// Fill out your copyright notice in the Description page of Project Settings.

#include "PhotoCharacter.h"
#include "Components/SkeletalMeshComponent.h"

// Sets default values
APhotoCharacter::APhotoCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bTickEvenWhenPaused = true;

	MaxLateralOffset = FVector(300.0f);
}

// Called when the game starts or when spawned
void APhotoCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (auto* mesh = GetMesh())
		mesh->SetTickableWhenPaused(true);
	photoCamera = FindComponentByClass<UCameraComponent>();
	springArm = FindComponentByClass<USpringArmComponent>();
	if (springArm.IsValid())
		springArm->SetTickableWhenPaused(true);

	CurrentPostProcessSettings.bOverride_VignetteIntensity = true;
	CurrentPostProcessSettings.bOverride_ColorGamma = true;
	CurrentPostProcessSettings.bOverride_ColorContrast = true;
	CurrentPostProcessSettings.bOverride_DepthOfFieldScale = true;
	CurrentPostProcessSettings.bOverride_DepthOfFieldFocalRegion = true;
	CurrentPostProcessSettings.bOverride_DepthOfFieldFocalDistance = true;
	tempDOFScale = dofScale;
	tempDOFRegion = dofRegion;
	tempVignettingValue = vignettingValue;
	tempRollValue = currentAddedRollValue;
	tempBrightnessValue = currentBrightnessValue;
}

// Called every frame
void APhotoCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!photoCamera.IsValid())
		return;

	if (interpDOFScale)
	{
		if (FMath::IsNearlyEqual(dofScale, tempDOFScale))
			interpDOFScale = false;
		tempDOFScale = FMath::FInterpTo(tempDOFScale, dofScale, DeltaTime, 5.0f);
	}
	if (interpDOFRegion)
	{
		if (FMath::IsNearlyEqual(dofRegion, tempDOFRegion))
			interpDOFRegion = false;
		tempDOFRegion = FMath::FInterpTo(tempDOFRegion, dofRegion, DeltaTime, 5.0f);
	}

	if (interpVignetting)
	{
		if (FMath::IsNearlyEqual(vignettingValue, tempVignettingValue))
			interpVignetting = false;
		tempVignettingValue = FMath::FInterpTo(tempVignettingValue, vignettingValue, DeltaTime, 5.0f);
	}
	if (interpBrightness)
	{
		if (FMath::IsNearlyEqual(currentBrightnessValue, tempBrightnessValue))
			interpBrightness = false;
		tempBrightnessValue = FMath::FInterpTo(tempBrightnessValue, currentBrightnessValue, DeltaTime, 5.0f);
	}
	if (interpRoll)
	{
		if (FMath::IsNearlyEqual(currentAddedRollValue, tempRollValue))
			interpRoll = false;
		tempRollValue = FMath::FInterpTo(tempRollValue, currentAddedRollValue, DeltaTime, 5.0f);
	}

	CurrentPostProcessSettings.DepthOfFieldFocalDistance = FVector::Dist(GetActorLocation(), photoCamera->GetComponentLocation());

	FVector4	brightness = 
		FVector4(1.0f, 1.0f, 1.0f, interpBrightness? tempBrightnessValue : currentBrightnessValue);
	CurrentPostProcessSettings.ColorGamma = brightness;
	CurrentPostProcessSettings.ColorContrast = brightness;
	CurrentPostProcessSettings.DepthOfFieldScale = interpDOFScale ? tempDOFScale : dofScale;
	CurrentPostProcessSettings.DepthOfFieldFocalRegion = interpDOFRegion ? tempDOFRegion : dofRegion;
	CurrentPostProcessSettings.VignetteIntensity = interpVignetting ? tempVignettingValue : vignettingValue;
	photoCamera->PostProcessSettings = CurrentPostProcessSettings;

	FRotator tempRot = photoCamera->GetComponentRotation();
	tempRot.Roll = interpRoll ? tempRollValue : currentAddedRollValue;
	photoCamera->SetWorldRotation(tempRot.Quaternion());
}

// Called to bind functionality to input
void APhotoCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void	APhotoCharacter::MoveForward(float value)
{
	if (!springArm.IsValid())
		return;
	FVector newValue = springArm->SocketOffset + FVector(value * GetWorld()->GetDeltaSeconds() * MoveSpeed * speedCoef, 0.0f, 0.0f);
	newValue.X = FMath::Clamp(newValue.X, -MaxLateralOffset.X, MaxLateralOffset.X);
	springArm->SocketOffset = newValue;
}

void	APhotoCharacter::MoveRight(float value)
{
	if (!springArm.IsValid())
		return;
	FVector newValue = springArm->SocketOffset + FVector(0.0f, value * GetWorld()->GetDeltaSeconds() * MoveSpeed * speedCoef, 0.0f);
	newValue.Y = FMath::Clamp(newValue.Y, -MaxLateralOffset.Y, MaxLateralOffset.Y);
	springArm->SocketOffset = newValue;
}

void	APhotoCharacter::MoveUp(float value)
{
	if (!springArm.IsValid())
		return;
	FVector newValue = springArm->SocketOffset + FVector(0.0f, 0.0f, value * GetWorld()->GetDeltaSeconds() * MoveSpeed * speedCoef);
	newValue.Z = FMath::Clamp(newValue.Z, -MaxLateralOffset.Z, MaxLateralOffset.Z);
	springArm->SocketOffset = newValue;
}

void	APhotoCharacter::RotateHorizontal(float value)
{
	AddControllerYawInput(value * RotateSpeed * GetWorld()->GetDeltaSeconds());
}

void	APhotoCharacter::RotateVertical(float value)
{
	AddControllerPitchInput(value * RotateSpeed * GetWorld()->GetDeltaSeconds());
}
		
void	APhotoCharacter::HideCharacter(bool value)
{
	SetActorHiddenInGame(value);
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

	if (photoCamera->FieldOfView < 170.0f)
		photoCamera->FieldOfView += 1.0f;
}

void	APhotoCharacter::DecreaseFOV()
{
	if (!photoCamera.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("APhotoCharacter : Couldn't decrease FOV because the cam is nullptr"));
		return;
	}

	if (photoCamera->FieldOfView > 5.0f)
		photoCamera->FieldOfView -= 1.0f;
}

void	APhotoCharacter::IncreaseCameraRoll()
{
	currentAddedRollValue += 1.0f;
	interpRoll = true;
}

void	APhotoCharacter::DecreaseCameraRoll()
{
	currentAddedRollValue -= 1.0f;
	interpRoll = true;
}

void	APhotoCharacter::IncreaseBrightness()
{ 
	currentBrightnessValue = FMath::Clamp(currentBrightnessValue + 0.1f, 0.1f, 2.2f); 
	interpBrightness = true;
}

void	APhotoCharacter::DecreaseBrightness()
{ 
	currentBrightnessValue = FMath::Clamp(currentBrightnessValue - 0.1f, 0.1f, 2.2f); 
	interpBrightness = true;
}

void	APhotoCharacter::ToggleDOF()
{
	bEnableDOF = !bEnableDOF;
	CurrentPostProcessSettings.bOverride_DepthOfFieldScale = bEnableDOF;
	CurrentPostProcessSettings.bOverride_DepthOfFieldFocalRegion = bEnableDOF;
	CurrentPostProcessSettings.bOverride_DepthOfFieldFocalDistance = bEnableDOF;
}

void	APhotoCharacter::DecreaseDOFScale()
{
	dofScale = FMath::Clamp(dofScale - 0.1f, DOFScaleRange.X, DOFScaleRange.Y);
	interpDOFScale = true;
}

void	APhotoCharacter::IncreaseDOFScale()
{
	dofScale = FMath::Clamp(dofScale + 0.1f, DOFScaleRange.X, DOFScaleRange.Y);
	interpDOFScale = true;
}

void	APhotoCharacter::DecreaseDOFRegion()
{
	dofRegion = FMath::Clamp(dofRegion - 50.0f, DOFRegionRange.X, DOFRegionRange.Y);
	interpDOFRegion = true;
}

void	APhotoCharacter::IncreaseDOFRegion()
{
	dofRegion = FMath::Clamp(dofRegion + 50.0f, DOFRegionRange.X, DOFRegionRange.Y);
	interpDOFRegion = true;
}

void	APhotoCharacter::DecreaseVignetting()
{
	vignettingValue = FMath::Clamp(vignettingValue - 0.1f, 0.0f, 1.0f);
	interpVignetting = true;
}

void	APhotoCharacter::IncreaseVignetting() 
{
	vignettingValue = FMath::Clamp(vignettingValue + 0.1f, 0.0f, 1.0f);
	interpVignetting = true;
}