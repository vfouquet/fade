// Fill out your copyright notice in the Description page of Project Settings.

#include "CameraRailManager.h"

#include "Engine/World.h"
#include "Components/SplineComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"

#include "Controllers/MainPlayerController.h"
#include "Cameras/RailCamera.h" 


ACameraRailManager::ACameraRailManager()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	SetRootComponent(RootComponent);

	SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComponent"));
	SplineComponent->SetupAttachment(RootComponent);
	SplineComponent->ClearSplinePoints();

	FlatSplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("FlatSplineComponent"));
	FlatSplineComponent->SetupAttachment(RootComponent);
	FlatSplineComponent->ClearSplinePoints();

	SplineComponent->SetWorldLocation(FVector::ZeroVector);
	FlatSplineComponent->SetWorldLocation(FVector::ZeroVector);

	bInCameraRailDistanceTolerance = false;
	CurrentInputKey = 0.0f;
}

float ACameraRailManager::GetInputKeyAtWorldLocation(FVector WorldLocation)
{
	return FlatSplineComponent->FindInputKeyClosestToWorldLocation(WorldLocation);
}

float ACameraRailManager::GetDistanceAlongSplineAtWorldLocation(FVector WorldLocation)
{
	float inputKey = this->FlatSplineComponent->FindInputKeyClosestToWorldLocation(WorldLocation);
	float inputKeyFloor = FMath::FloorToFloat(inputKey);

	float PointDistanceAlongSpline = this->FlatSplineComponent->GetDistanceAlongSplineAtSplinePoint(inputKeyFloor);
	float NextPointDistanceAlongSpline = this->FlatSplineComponent->GetDistanceAlongSplineAtSplinePoint(inputKeyFloor + 1);
	float DistanceBetweenTwoPoints = NextPointDistanceAlongSpline - PointDistanceAlongSpline;

	return PointDistanceAlongSpline + (DistanceBetweenTwoPoints * (inputKey - inputKeyFloor));
}

float ACameraRailManager::GetCurrentInputKey()
{
	return this->CurrentInputKey;
}

void ACameraRailManager::AttachCamera(ARailCamera* CameraToAttach)
{
	if (CameraToAttach != nullptr )
	{
		this->RailCamera = CameraToAttach;
		CurrentInputKey = FlatSplineComponent->FindInputKeyClosestToWorldLocation(PlayerActor->GetActorLocation());
		this->SetActorTickEnabled(true);
	}
}

void ACameraRailManager::AttachCamera(ARailCamera* CameraToAttach, AActor* PlayerActor)
{
	if (CameraToAttach != nullptr, PlayerActor != nullptr)
	{
		this->PlayerActor = PlayerActor;
		this->RailCamera = CameraToAttach;
		CurrentInputKey = FlatSplineComponent->FindInputKeyClosestToWorldLocation(PlayerActor->GetActorLocation());
		this->SetActorTickEnabled(true);
	}
}

void ACameraRailManager::DetachCamera()
{
	this->RailCamera = nullptr;
	this->SetActorTickEnabled(false);
}

void ACameraRailManager::PreEditChange(UProperty* PropertyThatWillChange)
{
#if WITH_EDITOR
	if (PropertyThatWillChange != nullptr && PropertyThatWillChange->GetNameCPP() == "RootPoint" && RootPoint != nullptr)
	{
		TAssetPtr<ACameraRailPoint>	CurrentPoint = RootPoint;

		while (CurrentPoint.IsValid() && CurrentPoint->PreviousPoint.IsValid())
		{
			CurrentPoint = CurrentPoint->PreviousPoint.Get();
		}

		TAssetPtr<ACameraRailPoint> RootPoint = CurrentPoint;

		while (CurrentPoint.IsValid())
		{
			CurrentPoint->CurrentCameraRailManager = nullptr;
			CurrentPoint = CurrentPoint->NextPoint.Get();
		}
	}
#endif
}

void ACameraRailManager::PostEditMove(bool bFinished)
{
#if WITH_EDITOR
	if (SplineComponent != nullptr && FlatSplineComponent != nullptr)
	{
		SplineComponent->SetWorldLocation(FVector::ZeroVector);
		FlatSplineComponent->SetWorldLocation(FVector::ZeroVector);
	}
#endif
}

void ACameraRailManager::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

#if WITH_EDITOR
	UpdateSpline(); 
#endif
}

void ACameraRailManager::BeginPlay()
{
	Super::BeginPlay();

#if WITH_EDITOR
	UpdateSpline();
#endif

	AMainPlayerController* PC = Cast<AMainPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));

	if (PC != nullptr)
	{
		PlayerActor = PC->GetPawn();
	}

	this->SetActorTickEnabled(false);
}

void ACameraRailManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (RailCamera != nullptr && PlayerActor != nullptr)
	{
		if (bInCameraRailDistanceTolerance == false)
		{
			FVector PlayerLoc = PlayerActor->GetActorLocation();

			float NewCurrentInputKey = FMath::FInterpConstantTo(CurrentInputKey, FlatSplineComponent->FindInputKeyClosestToWorldLocation(PlayerLoc), DeltaSeconds, RailCamera->CameraSettings.CameraSpeed);
			FVector NextCameraLocation = SplineComponent->GetLocationAtSplineInputKey(NewCurrentInputKey, ESplineCoordinateSpace::World);
			FVector NextCameraLocationWithOffset = SplineComponent->GetLocationAtSplineInputKey(NewCurrentInputKey + RailCamera->CameraSettings.CameraRailOffset, ESplineCoordinateSpace::World);

			FVector	CentroidRelativeLocFromPlayer = FVector::ZeroVector;

			for (FCameraInterestPoint point : RailCamera->CameraSettings.InterestPoints)
			{
				if (point.Actor.IsValid())
				{
					FVector PointRelactiveLocFromPlayer = point.Actor->GetActorLocation() - PlayerActor->GetActorLocation();
					PointRelactiveLocFromPlayer *= point.Weight;
					CentroidRelativeLocFromPlayer += PointRelactiveLocFromPlayer;
				}
			}

			CentroidRelativeLocFromPlayer /= RailCamera->CameraSettings.InterestPoints.Num() + 1;

 			FVector LookAtDir = (PlayerActor->GetActorLocation() + CentroidRelativeLocFromPlayer) - NextCameraLocationWithOffset;

			RailCamera->SetActorLocation(NextCameraLocation);
			RailCamera->GetCameraArm()->SetWorldLocation(NextCameraLocationWithOffset);
			FRotator TargetRotation = FMath::RInterpConstantTo(RailCamera->GetCameraArm()->GetComponentRotation(), FRotationMatrix::MakeFromX(LookAtDir).Rotator(), DeltaSeconds, RailCamera->CameraSettings.CameraRotationSpeed);

			RailCamera->GetCameraArm()->SetWorldRotation(TargetRotation);

			if (NewCurrentInputKey == this->CurrentInputKey)
			{
				bInCameraRailDistanceTolerance = true;
			}

			this->CurrentInputKey = NewCurrentInputKey;
		}
		else
		{
			FVector PlayerLoc = PlayerActor->GetActorLocation();

			float NewCurrentInputKey = FMath::FInterpConstantTo(CurrentInputKey, FlatSplineComponent->FindInputKeyClosestToWorldLocation(PlayerLoc), DeltaSeconds, RailCamera->CameraSettings.CameraSpeed);
			FVector NextCameraLocation = SplineComponent->GetLocationAtSplineInputKey(NewCurrentInputKey, ESplineCoordinateSpace::World);
			FVector NextCameraLocationWithOffset = SplineComponent->GetLocationAtSplineInputKey(NewCurrentInputKey + RailCamera->CameraSettings.CameraRailOffset, ESplineCoordinateSpace::World);

			FVector	CentroidRelativeLocFromPlayer = FVector::ZeroVector;

			for (FCameraInterestPoint point : RailCamera->CameraSettings.InterestPoints)
			{
				if (point.Actor.IsValid())
				{
					FVector PointRelactiveLocFromPlayer = point.Actor->GetActorLocation() - PlayerActor->GetActorLocation();
					PointRelactiveLocFromPlayer *= point.Weight;
					CentroidRelativeLocFromPlayer += PointRelactiveLocFromPlayer;
				}
			}

			CentroidRelativeLocFromPlayer /= RailCamera->CameraSettings.InterestPoints.Num() + 1;

			FVector LookAtDir = (PlayerActor->GetActorLocation() + CentroidRelativeLocFromPlayer) - RailCamera->GetActorLocation(); //

			//RailCamera->SetActorLocation(NextCameraLocation);

			//RailCamera->GetCameraArm()->SetWorldLocation(NextCameraLocationWithOffset);
			FRotator TargetRotation = FMath::RInterpConstantTo(RailCamera->GetCameraArm()->GetComponentRotation(), FRotationMatrix::MakeFromX(LookAtDir).Rotator(), DeltaSeconds, RailCamera->CameraSettings.CameraRotationSpeed);

			RailCamera->GetCameraArm()->SetWorldRotation(TargetRotation);

			this->CurrentInputKey = NewCurrentInputKey;

			float DistanceBetweenCameraAndCurrentInputKey = GetDistanceAlongSplineAtWorldLocation(RailCamera->GetActorLocation()) - GetDistanceAlongSplineAtWorldLocation(NextCameraLocation);


			if (DistanceBetweenCameraAndCurrentInputKey > RailCamera->CameraSettings.CameraRailDistanceTolerance
				|| DistanceBetweenCameraAndCurrentInputKey < -RailCamera->CameraSettings.CameraRailDistanceTolerance)
			{
				bInCameraRailDistanceTolerance = false;
				this->CurrentInputKey = FlatSplineComponent->FindInputKeyClosestToWorldLocation(RailCamera->GetActorLocation());
			}
		}
		
	}
}

void ACameraRailManager::UpdateSpline()
{
	if (RootPoint != nullptr && SplineComponent != nullptr && FlatSplineComponent != nullptr)
	{
		float InputKey = 0.0f;

		SplineComponent->ClearSplinePoints();
		FlatSplineComponent->ClearSplinePoints();
		TAssetPtr<ACameraRailPoint>	CurrentPoint = RootPoint;

		while (CurrentPoint.IsValid() && CurrentPoint->PreviousPoint.IsValid())
		{
			CurrentPoint = CurrentPoint->PreviousPoint.Get();
		}

		TAssetPtr<ACameraRailPoint> RootPoint = CurrentPoint;

		while(CurrentPoint.IsValid())
		{
			CurrentPoint->CurrentCameraRailManager = this;
			CurrentPoint->SplinePoint.InputKey = 0.0f;
			CurrentPoint = CurrentPoint->NextPoint.Get();
		}

		CurrentPoint = RootPoint;

		while (CurrentPoint.IsValid() && CurrentPoint->NextPoint.IsValid()) 
		{
			if (CurrentPoint->SplinePoint.InputKey == 0.0f)
			{
				CurrentPoint->SplinePoint.InputKey = InputKey;
				SplineComponent->AddPoint(CurrentPoint->SplinePoint);

				FSplinePoint FlatSplinePoint(CurrentPoint->SplinePoint.InputKey,
					CurrentPoint->SplinePoint.Position,
					CurrentPoint->SplinePoint.ArriveTangent,
					CurrentPoint->SplinePoint.LeaveTangent,
					CurrentPoint->SplinePoint.Rotation,
					CurrentPoint->SplinePoint.Scale,
					CurrentPoint->SplinePoint.Type);

				FlatSplinePoint.Position.Z = 0.0f;
				FlatSplineComponent->AddPoint(FlatSplinePoint);
				++InputKey;
			}
			CurrentPoint = CurrentPoint->NextPoint.Get();
		}

		if (CurrentPoint.IsValid() && CurrentPoint->SplinePoint.InputKey == 0.0f)
		{
			CurrentPoint->SplinePoint.InputKey = InputKey;
			SplineComponent->AddPoint(CurrentPoint->SplinePoint);

			FSplinePoint FlatSplinePoint(CurrentPoint->SplinePoint.InputKey,
				CurrentPoint->SplinePoint.Position,
				CurrentPoint->SplinePoint.ArriveTangent,
				CurrentPoint->SplinePoint.LeaveTangent,
				CurrentPoint->SplinePoint.Rotation,
				CurrentPoint->SplinePoint.Scale,
				CurrentPoint->SplinePoint.Type);

			FlatSplinePoint.Position.Z = 0.0f;
			FlatSplineComponent->AddPoint(FlatSplinePoint);
		}
	}
}

ACameraRailPoint* ACameraRailManager::GetCameraPointAtInputKey(float InputKey)
{
	ACameraRailPoint* point = RootPoint.Get();

	float RoundedInputKey = FMath::RoundToNegativeInfinity(InputKey);

	while (point != nullptr)
	{
		if (RoundedInputKey == point->SplinePoint.InputKey)
		{
			return point;
		}

		point = point->NextPoint.Get();
	}

	return point;
}

