// Fill out your copyright notice in the Description page of Project Settings.

#include "MainPlayerController.h"

#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/SpectatorPawn.h"
#include "GameFramework/PlayerState.h"

#include "Cameras/CameraRailManager.h"

#include "MainCharacter.h"
#include "AkAudio/Classes/AkGameplayStatics.h"

#include "Camera/CameraActor.h"
#include "MainSpectatorPawn.h"
#include "BreathGameInstance.h"

void AMainPlayerController::BeginPlay()
{
	Super::BeginPlay();

	UBreathGameInstance* gameInst = Cast<UBreathGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (gameInst)
		bIsTPS = gameInst->IsCameraTPS();
	if (bIsTPS)
		return;

	this->bAutoManageActiveCameraTarget = true;

	if (CameraActorClass != nullptr)
	{
		CameraActor = GetWorld()->SpawnActor<ACameraActor>(CameraActorClass, this->GetActorTransform());
	}
	else
	{
		CameraActor = GetWorld()->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), this->GetActorTransform());
	}

#if WITH_EDITOR
	CameraActor->SetActorLabel(TEXT("RailCamera"));
#endif
	this->SetViewTarget(CameraActor);

}

void AMainPlayerController::SetSpectatorPawn(class ASpectatorPawn* NewSpectatorPawn)
{
	Super::SetSpectatorPawn(NewSpectatorPawn);

	if (NewSpectatorPawn != nullptr && this->MainCharacter != nullptr)
	{
		NewSpectatorPawn->SetActorTransform(this->MainCharacter->GetActorTransform());
	}
}

void AMainPlayerController::DebugMode(bool bValue)
{
	if (bValue)
	{
		this->ChangeState(NAME_Spectating);
		this->PlayerState->bIsSpectator = true;

		AMainSpectatorPawn* specPawn = Cast<AMainSpectatorPawn>(GetSpectatorPawn());
		if (specPawn)
			specPawn->SetBasicMode();
	}
	else
	{
		this->PlayerState->bIsSpectator = false;
		this->ChangeState(NAME_Playing);
		Possess(MainCharacter);
	}
}

void	AMainPlayerController::PhotoMode(bool bValue, bool pauseGame)
{
	if (bValue)
	{
		if (MainCharacter)
			MainCharacter->SetActorHiddenInGame(true);

		if (PhotoCharacterClass)
		{
			if (MainCharacter)
			{
				FActorSpawnParameters	spParams;
				spParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				PhotoCharacter = GetWorld()->SpawnActor<APhotoCharacter>(PhotoCharacterClass.Get(), MainCharacter->GetTransform(), spParams);
				if (PhotoCharacter.IsValid())
				{
					MainCharacter->GetMesh()->SnapshotPose(PhotoCharacter->GetSnapshotPose());
					Possess(PhotoCharacter.Get());
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("AMainPlayerController : Couldn't create photo character because the main is nullptr"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("AMainPlayerController : Couldn't create PhotoCharacter because the class is not set"));
		}

		if (pauseGame)
			UGameplayStatics::SetGamePaused(GetWorld(), true);
	}
	else
	{
		if (PhotoCharacter.IsValid())
			PhotoCharacter->Destroy();

		if (MainCharacter)
			MainCharacter->SetActorHiddenInGame(false);

		if (pauseGame)
			UGameplayStatics::SetGamePaused(GetWorld(), false);
		Possess(MainCharacter);
	}
}

ACameraActor* AMainPlayerController::GetCameraActor()
{
	return CameraActor;
}

void AMainPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (InputComponent != nullptr)
	{
		InputComponent->BindAxis("MoveForward", this, &AMainPlayerController::MoveForward);
		InputComponent->BindAxis("MoveRight", this, &AMainPlayerController::MoveRight);
		InputComponent->BindAxis("MoveForward", this, &AMainPlayerController::MovePhotoForward).bExecuteWhenPaused = true;
		InputComponent->BindAxis("MoveRight", this, &AMainPlayerController::MovePhotoRight).bExecuteWhenPaused = true;
		InputComponent->BindAxis("LookHorizontal", this, &AMainPlayerController::RotateHorizontal);
		InputComponent->BindAxis("LookVertical", this, &AMainPlayerController::RotateVertical);

		InputComponent->BindAction("Jump", IE_Pressed, this, &AMainPlayerController::Jump).bExecuteWhenPaused = false;
		InputComponent->BindAction("Action", IE_Pressed, this, &AMainPlayerController::Action).bExecuteWhenPaused = false;
		InputComponent->BindAction("Grab", IE_Pressed, this, &AMainPlayerController::BeginGrab).bExecuteWhenPaused = false;
		InputComponent->BindAction("Grab", IE_Released, this, &AMainPlayerController::StopGrab).bExecuteWhenPaused = false;
		InputComponent->BindAction("Pause", IE_Pressed, this, &AMainPlayerController::Pause).bExecuteWhenPaused = true;

		InputComponent->BindAction("MenuUp", IE_Pressed, this, &AMainPlayerController::MenuUp).bExecuteWhenPaused = true;
		InputComponent->BindAction("MenuDown", IE_Pressed, this, &AMainPlayerController::MenuDown).bExecuteWhenPaused = true;
		InputComponent->BindAction("MenuRight", IE_Pressed, this, &AMainPlayerController::MenuRight).bExecuteWhenPaused = true;
		InputComponent->BindAction("MenuLeft", IE_Pressed, this, &AMainPlayerController::MenuLeft).bExecuteWhenPaused = true;
		InputComponent->BindAction("MenuValidate", IE_Pressed, this, &AMainPlayerController::MenuValidatePressed).bExecuteWhenPaused = true;
		InputComponent->BindAction("MenuValidate", IE_Released, this, &AMainPlayerController::MenuValidateReleased).bExecuteWhenPaused = true;
		InputComponent->BindAction("MenuCancel", IE_Pressed, this, &AMainPlayerController::MenuBackPressed).bExecuteWhenPaused = true;
		InputComponent->BindAction("MenuCancel", IE_Released, this, &AMainPlayerController::MenuBackReleased).bExecuteWhenPaused = true;
		InputComponent->BindAction("MenuSpecial", IE_Released, this, &AMainPlayerController::MenuSpecialButton).bExecuteWhenPaused = true;
#if WITH_EDITOR
		InputComponent->BindAction("DebugPauseEditor", IE_Pressed, this, &AMainPlayerController::DebugPauseEditor);
#endif
	}
}

void AMainPlayerController::Possess(APawn* aPawn)
{
	Super::Possess(aPawn);

	AMainCharacter* tempChara = Cast<AMainCharacter>(aPawn);
	if (tempChara)
		MainCharacter = tempChara;

	if (tempChara != nullptr)
	{
		if (!bIsTPS)
			this->SetViewTarget(CameraActor);

		SpringArmComponent = MainCharacter->FindComponentByClass<USpringArmComponent>();

		
		FScriptDelegate	onDieDel;
		onDieDel.BindUFunction(this, "OnPawnDeath");
		MainCharacter->OnDie.Add(onDieDel);

	}
}

void AMainPlayerController::UnPossess()
{
	Super::UnPossess();

	//SpringArmComponent = nullptr;
	//MainCharacter = nullptr;
}


void	AMainPlayerController::MoveForward(float Value)
{
	if (this->GetSpectatorPawn() == nullptr && MainCharacter != nullptr)
	{
		lastStickInput.Y = Value;
		updateCharacterValues();

		float stickLength = lastStickInput.Size();
		if (stickLength < WalkStickThreshold)
		{
			MainCharacter->Move(FVector::ZeroVector);
			return;
		}

		FRotator CamRot = GetCameraRotation();
		CamRot.Pitch = 0.0f;
		FVector MoveDir = CamRot.Vector();
		MainCharacter->Move(MoveDir * Value);
	}
	else if (auto* spec = Cast<AMainSpectatorPawn>(this->GetSpectatorPawn()))
		spec->MoveForward(Value);
}

void	AMainPlayerController::MoveRight(float Value)
{
	if (this->GetSpectatorPawn() == nullptr && MainCharacter != nullptr)
	{
		lastStickInput.X = Value;
		float stickLength = lastStickInput.Size();
		updateCharacterValues();
		if (stickLength < WalkStickThreshold && stickLength)
		{
			MainCharacter->Move(FVector::ZeroVector);
			return;
		}

		FRotator CamRot = GetCameraRotation();
		CamRot.Pitch = 0.0f;
		FVector MoveDir = CamRot.RotateVector(FVector::RightVector);
		MainCharacter->Move(MoveDir * Value);
	}
	else if (auto* spec = Cast<AMainSpectatorPawn>(this->GetSpectatorPawn()))
		spec->MoveRight(Value);
}

void	AMainPlayerController::MovePhotoForward(float value)
{
	if (auto* spec = Cast<AMainSpectatorPawn>(this->GetSpectatorPawn()))
		spec->MoveForward(value);
}

void	AMainPlayerController::MovePhotoRight(float value)
{
	if (auto* spec = Cast<AMainSpectatorPawn>(this->GetSpectatorPawn()))
		spec->MoveRight(value);
}

void	AMainPlayerController::RotateHorizontal(float Value)
{
	if (this->GetSpectatorPawn() == nullptr && MainCharacter != nullptr)
		MainCharacter->RotateHorizontal(Value);
	else if (auto* spec = Cast<AMainSpectatorPawn>(this->GetSpectatorPawn()))
		spec->RotateHorizontal(Value);
	if (PhotoCharacter.IsValid())
		PhotoCharacter->RotateHorizontal(Value);
}

void	AMainPlayerController::RotateVertical(float Value)
{
	if (this->GetSpectatorPawn() == nullptr && MainCharacter != nullptr)
		MainCharacter->RotateVertical(Value);
	else if (auto* spec = Cast<AMainSpectatorPawn>(this->GetSpectatorPawn()))
		spec->RotateVertical(Value);
	if (PhotoCharacter.IsValid())
		PhotoCharacter->RotateVertical(Value);
}

void	AMainPlayerController::Pause()
{
	if (UGameplayStatics::IsGamePaused(GetWorld()))
	{
		if (currentUIWidget.IsValid())
		{
			currentUIWidget->OnUnpause();
			IUMGController::Execute_OnUnpause(currentUIWidget.Get());
			currentUIWidget->RemoveFromParent();
			currentUIWidget = nullptr;
		}
		SetInputMode(FInputModeGameOnly());
		UAkGameplayStatics::PostEvent(ResumeAllEvent, this);
		UGameplayStatics::SetGamePaused(GetWorld(), false);
		bIsPaused = false;
	}
	else
	{
		if (!PauseWidgetSample)
		{
			UE_LOG(LogTemp, Error, TEXT("AMainPlayerController : Couldn't create PauseUI beacuse the sample is not defined"));
			return;
		}

		currentUIWidget = CreateWidget<UUIWidgetControllerSupported>(this, PauseWidgetSample);
		if (!currentUIWidget.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("AMainPlayerController : PauseUI is nullptr after creation"));
			return;
		}
		currentUIWidget->AddToViewport();
		UAkGameplayStatics::PostEvent(PauseAllEvent, this);
		UGameplayStatics::SetGamePaused(GetWorld(), true);
		bIsPaused = true;
	}
}

void	AMainPlayerController::MenuUp()
{
	if (currentUIWidget.IsValid() && bIsPaused)
		IUMGController::Execute_MoveUp(currentUIWidget.Get());
}

void	AMainPlayerController::MenuDown()
{
	if (currentUIWidget.IsValid() && bIsPaused)
		IUMGController::Execute_MoveDown(currentUIWidget.Get());
}

void	AMainPlayerController::MenuLeft()
{
	if (currentUIWidget.IsValid() && bIsPaused)
		IUMGController::Execute_MoveLeft(currentUIWidget.Get());
}

void	AMainPlayerController::MenuRight()
{
	if (currentUIWidget.IsValid() && bIsPaused)
		IUMGController::Execute_MoveRight(currentUIWidget.Get());
}

void	AMainPlayerController::MenuValidatePressed()
{
	if (currentUIWidget.IsValid() && bIsPaused)
		IUMGController::Execute_Validate(currentUIWidget.Get(), true);
}

void	AMainPlayerController::MenuValidateReleased()
{
	if (currentUIWidget.IsValid() && bIsPaused)
		IUMGController::Execute_Validate(currentUIWidget.Get(), false);
}

void	AMainPlayerController::MenuBackPressed()
{
	if (currentUIWidget.IsValid() && bIsPaused)
		IUMGController::Execute_Cancel(currentUIWidget.Get(), true);
}

void	AMainPlayerController::MenuBackReleased()
{
	if (currentUIWidget.IsValid() && bIsPaused)
		IUMGController::Execute_Cancel(currentUIWidget.Get(), false);
}

void	AMainPlayerController::MenuSpecialButton()
{
	if (currentUIWidget.IsValid() && bIsPaused)
		IUMGController::Execute_SpecialButton(currentUIWidget.Get());
}

void AMainPlayerController::Jump()
{
	if (bIsPaused)	return;

	if (this->GetSpectatorPawn() == nullptr && MainCharacter != nullptr)
	{	
		if (MainCharacter->CanThrow())
			MainCharacter->Throw();
		else
			MainCharacter->Jump();
	}
}

void	AMainPlayerController::Action()
{
	if (bIsPaused)	return;
	
	if (this->GetSpectatorPawn() == nullptr && MainCharacter)
		MainCharacter->Action();
}

void	AMainPlayerController::BeginGrab()
{
	if (bIsPaused)	return;
	
	if (this->GetSpectatorPawn() == nullptr && MainCharacter != nullptr)
		MainCharacter->BeginGrab();
}

void	AMainPlayerController::StopGrab()
{
	if (bIsPaused)	return;
	
	if (this->GetSpectatorPawn() == nullptr && MainCharacter != nullptr)
		MainCharacter->StopGrab();
}

void	AMainPlayerController::EnableGodMode()
{
	if (MainCharacter != nullptr)
		MainCharacter->SetGodMode(true);
}

void	AMainPlayerController::DisableGodMode()
{
	if (MainCharacter != nullptr)
		MainCharacter->SetGodMode(false);
}

#if WITH_EDITOR
#include "UnrealEd.h"
void	AMainPlayerController::DebugPauseEditor()
{
	GUnrealEd->PlayWorld->bDebugPauseExecution = true;
}
#endif
	
void	AMainPlayerController::updateCharacterValues()
{
	float stickLength = lastStickInput.Size();
	if (stickLength <= JogStickThreshold)
		MainCharacter->SetWalkMode();
	else
		MainCharacter->SetJogMode();

	if (stickLength < WalkStickThreshold)
		MainCharacter->SetCanAutoClimb(false);
	else
	{
		//FRotator stickRot = FRotationMatrix::MakeFromX(FVector(GetInputAxisValue("MoveForward"), GetInputAxisValue("MoveRight"), 0.0f)).Rotator();
		//float angle = FMath::RadiansToDegrees(FMath::Acos(MainCharacter->GetActorForwardVector() | stickRot.Vector()));
		//UE_LOG(LogTemp, Warning, TEXT("%f"), MainCharacter->GetActorForwardVector() | stickRot.Vector());
		MainCharacter->SetCanAutoClimb(true);
		//MainCharacter->SetCanAutoClimb(angle > ClimbStickAngleTolerence);
	}

	if (FMath::IsNearlyZero(stickLength))
	{
		MainCharacter->SetRotatingLeft(false);
		MainCharacter->SetRotatingRight(false);
		MainCharacter->SetPushingAxis(0.0f);
		return;
	}

	float	inputAngle = FMath::RadiansToDegrees(FMath::Atan2(GetInputAxisValue("MoveForward"), GetInputAxisValue("MoveRight")));
	float cameraDiffAngle = GetCameraRotation().Yaw - MainCharacter->GetActorRotation().Yaw;

	inputAngle += 90.0f;
	if (inputAngle < 0.0f)
		inputAngle += 360.0f;

	if (cameraDiffAngle < 0.0f)
		cameraDiffAngle += 360.0f;

	float difference = inputAngle - cameraDiffAngle;

	if (difference < 0.0f)
		difference += 360.0f;

	if (difference >= 180.0f - HeavyAngleTolerance && difference <= 180.0f + HeavyAngleTolerance)
	{
		MainCharacter->SetRotatingLeft(false);
		MainCharacter->SetRotatingRight(false);
		MainCharacter->SetPushingAxis(1.0f);
	}
	else if (difference >= 360.0f - HeavyAngleTolerance || difference <= HeavyAngleTolerance)
	{
		MainCharacter->SetRotatingLeft(false);
		MainCharacter->SetRotatingRight(false);
		MainCharacter->SetPushingAxis(-1.0f);
	}
	else
	{
		MainCharacter->SetRotatingRight(difference > HeavyAngleTolerance && difference < 180.0f - HeavyAngleTolerance);
		MainCharacter->SetRotatingLeft(difference > 180.0f + HeavyAngleTolerance && difference < 360.0f - HeavyAngleTolerance);
		MainCharacter->SetPushingAxis(0.0f);
	}
}