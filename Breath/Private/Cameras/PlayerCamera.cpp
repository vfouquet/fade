// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCamera.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

#include "ViewTargetComponent.h"

APlayerCamera::APlayerCamera()
{
	PrimaryActorTick.bCanEverTick = true;

	TargetTransitionTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("MyTimeline"));
}

void APlayerCamera::BeginPlay()
{
	Super::BeginPlay();

	if (TargetTransitionCurve != nullptr)
	{
		FOnTimelineFloat callback{};
		FOnTimelineEvent endCallback{};

		callback.BindUFunction(this, FName{ TEXT("TargetTransitionCallback") });
		endCallback.BindUFunction(this, FName{ TEXT("TargetTransitionEndCallback") });

		TargetTransitionTimeline->AddInterpFloat(TargetTransitionCurve, callback, FName{ TEXT("TargetTransitionAnimation") });
		TargetTransitionTimeline->SetTimelineFinishedFunc(endCallback);
	}

	PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);

	AttachToTarget(PlayerCharacter);
}

void APlayerCamera::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (ViewTarget != nullptr)
	{
		this->SetActorLocationAndRotation(ViewTarget->GetComponentLocation(), ViewTarget->GetComponentRotation());
	}
}

void APlayerCamera::AttachToTarget(AActor* NewTarget, bool bTeleport)
{
	Target = NewTarget;
	ViewTarget = nullptr;

	if (Target != nullptr)
	{
		ViewTarget = Target->FindComponentByClass<UViewTargetComponent>();

		if (ViewTarget == nullptr)
		{
			ViewTarget = Target->GetRootComponent();
		}

		if (bTeleport == false && TargetTransitionCurve != nullptr)
		{
			TargetTransitionStartTransform = this->GetTransform();
			TargetTransitionTimeline->PlayFromStart();
			SetActorTickEnabled(false);
		}
	}
}

void APlayerCamera::TargetTransitionCallback(float StepValue)
{
		FVector interpPosition = FMath::Lerp(TargetTransitionStartTransform.GetLocation(), ViewTarget->GetComponentLocation(), StepValue);
		FQuat	interpQuat = FMath::Lerp(TargetTransitionStartTransform.GetRotation(), ViewTarget->GetComponentQuat(), StepValue);

		this->SetActorLocationAndRotation(interpPosition, interpQuat);
}

void APlayerCamera::TargetTransitionEndCallback()
{
	SetActorTickEnabled(true);
}
