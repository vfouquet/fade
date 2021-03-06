// Fill out your copyright notice in the Description page of Project Settings.

#include "CameraRailManager.h"

#include "Engine/World.h"
#include "Components/SplineComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "Components/PrimitiveComponent.h"

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

	float PointDistanceAlongSpline = this->SplineComponent->GetDistanceAlongSplineAtSplinePoint(inputKeyFloor);
	float NextPointDistanceAlongSpline = this->SplineComponent->GetDistanceAlongSplineAtSplinePoint(inputKeyFloor + 1);
	float DistanceBetweenTwoPoints = NextPointDistanceAlongSpline - PointDistanceAlongSpline;

	return PointDistanceAlongSpline + (DistanceBetweenTwoPoints * (inputKey - inputKeyFloor));
}

float ACameraRailManager::GetCurrentInputKey()
{
	return this->CurrentInputKey;
}

float ACameraRailManager::GetCurrentDistanceAlongSpline()
{
	return this->CurrentDistanceAlongSpline;
}

void ACameraRailManager::AttachCamera(ARailCamera* CameraToAttach)
{
	if (CameraToAttach != nullptr )
	{
		this->RailCamera = CameraToAttach;
	}
}

void ACameraRailManager::AttachCamera(ARailCamera* CameraToAttach, AActor* PlayerActor, bool bTeleport)
{
	if (CameraToAttach != nullptr)
	{
		this->RailCamera = CameraToAttach;
		
		this->ChangePlayer(PlayerActor, bTeleport);
	}
}

void	ACameraRailManager::ChangePlayer(AActor* PlayerActor, bool bTeleport)
{
	if (this->RailCamera != nullptr && PlayerActor != nullptr)
	{
		this->PlayerActor = PlayerActor;
		this->CurrentInputKey = FlatSplineComponent->FindInputKeyClosestToWorldLocation(PlayerActor->GetActorLocation());

		if (bTeleport == true)
		{
			this->CurrentDistanceAlongSpline = GetDistanceAlongSplineAtWorldLocation(PlayerActor->GetActorLocation());
			this->CurrentDistanceAlongSplineWithOffset = this->CurrentDistanceAlongSpline + this->RailCamera->CameraSettings.CameraRailOffset;
			this->LastDistanceAlongSplineTarget = this->CurrentDistanceAlongSpline;
			FVector NextCameraLocation = SplineComponent->GetWorldLocationAtDistanceAlongSpline(this->CurrentDistanceAlongSpline);
			FVector NextCameraLocationWithOffset = SplineComponent->GetWorldLocationAtDistanceAlongSpline(this->CurrentDistanceAlongSplineWithOffset);

			// Camera focus
			FVector	CentroidRelativeLocFromPlayer = FVector::ZeroVector;
			for (FCameraInterestPoint point : RailCamera->CameraSettings.InterestPoints)
			{
				if (point.Actor.IsValid())
				{
					FVector PointRelactiveLocFromPlayer = point.Actor->GetActorLocation() - PlayerActor->GetRootPrimitiveComponent()->GetComponentToWorld().GetLocation();
					PointRelactiveLocFromPlayer *= point.Weight;
					CentroidRelativeLocFromPlayer += PointRelactiveLocFromPlayer;
				}
			}

			CentroidRelativeLocFromPlayer /= RailCamera->CameraSettings.InterestPoints.Num() + 1; // 1 represents the player weight

			// Sets new camera location
			RailCamera->SetActorLocation(NextCameraLocation);
			RailCamera->GetCameraArm()->SetWorldLocation(NextCameraLocationWithOffset);


			// Computes and sets new camera rotation

			FVector LookAtDir = (PlayerActor->GetActorLocation() + CentroidRelativeLocFromPlayer) - NextCameraLocationWithOffset + RailCamera->GetCameraComponent()->RelativeLocation;
			FVector LookAtDirRail = (PlayerActor->GetActorLocation() + CentroidRelativeLocFromPlayer) - RailCamera->GetCameraArm()->GetComponentLocation();
			FRotator TargetRotationRail = FRotationMatrix::MakeFromX(LookAtDirRail).Rotator();

			RailCamera->GetCameraArm()->SetWorldRotation(FRotator::MakeFromEuler(FVector(0.f, 0.f, TargetRotationRail.Yaw)));
			RailCamera->GetCameraComponent()->SetWorldRotation(FRotationMatrix::MakeFromX(LookAtDir).Rotator());
		}

		this->SetActorTickEnabled(true);
	}
}

void ACameraRailManager::DetachCamera()
{
	this->RailCamera = nullptr;
	this->SetActorTickEnabled(false);
}

#if WITH_EDITOR
void ACameraRailManager::PreEditChange(UProperty* PropertyThatWillChange)
{
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
}
#endif

#if WITH_EDITOR
void ACameraRailManager::PostEditMove(bool bFinished)
{
	if (SplineComponent != nullptr && FlatSplineComponent != nullptr)
	{
		SplineComponent->SetWorldLocation(FVector::ZeroVector);
		FlatSplineComponent->SetWorldLocation(FVector::ZeroVector);
	}
}
#endif

#if WITH_EDITOR
void ACameraRailManager::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	UpdateSpline(); 
}
#endif

//#if WITH_EDITOR
void ACameraRailManager::BeginPlay()
{
	Super::BeginPlay();

	UpdateSpline();

	/*AMainPlayerController* PC = Cast<AMainPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));

	if (PC != nullptr)
	{
		PlayerActor = PC->GetPawn();
	}*/

	if (this->RailCamera == nullptr)
	{
		this->SetActorTickEnabled(false);
	}
}
//#endif


void ACameraRailManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (RailCamera != nullptr && PlayerActor != nullptr)
	{
		FVector PlayerLoc = PlayerActor->GetActorLocation();

		RailCamera->UpdateCamera(DeltaSeconds);

		this->LastDistanceAlongSplineTarget = GetDistanceAlongSplineAtWorldLocation(PlayerLoc);

		// Computes new distance along spline and new world location of the camera
		float NewDistanceAlongSpline = FMath::FInterpConstantTo(CurrentDistanceAlongSpline, this->LastDistanceAlongSplineTarget, DeltaSeconds, RailCamera->CurrentCameraSettings.CameraSpeed);
		float NewDistanceAlongSplineWithOffset = FMath::FInterpConstantTo(CurrentDistanceAlongSplineWithOffset, this->LastDistanceAlongSplineTarget + RailCamera->CurrentCameraSettings.CameraRailOffset, DeltaSeconds, RailCamera->CurrentCameraSettings.CameraSpeed);
		FVector NextCameraLocation = SplineComponent->GetWorldLocationAtDistanceAlongSpline(NewDistanceAlongSpline);
		FVector NextCameraLocationWithOffset = SplineComponent->GetWorldLocationAtDistanceAlongSpline(NewDistanceAlongSplineWithOffset);

		float NewCurrentInputKey = FMath::FInterpConstantTo(CurrentInputKey, FlatSplineComponent->FindInputKeyClosestToWorldLocation(PlayerLoc), DeltaSeconds, RailCamera->CameraSettings.CameraSpeed);

		// Camera focus
		FVector	CentroidRelativeLocFromPlayer = FVector::ZeroVector;

		for (FCameraInterestPoint point : RailCamera->CameraSettings.InterestPoints)
		{	
			if (point.Actor.IsValid())
			{
				FVector PointRelactiveLocFromPlayer = point.Actor->GetActorLocation() - PlayerActor->GetRootPrimitiveComponent()->GetComponentToWorld().GetLocation();
				PointRelactiveLocFromPlayer *= point.Weight;
				CentroidRelativeLocFromPlayer += PointRelactiveLocFromPlayer;
			}
		}

		CentroidRelativeLocFromPlayer /= RailCamera->CameraSettings.InterestPoints.Num() + 1; // 1 represents the player weight.

		FVector LookAtDir;

		// Camera tolerance (Camera don't follows the player until he reaches a certain distance from the camera)
		// Out tolerance
		if (bInCameraRailDistanceTolerance == false)
		{
//  			LookAtDir = (PlayerActor->GetActorLocation() + CentroidRelativeLocFromPlayer) - (NextCameraLocationWithOffset + RailCamera->GetCameraComponent()->RelativeLocation);

			// Sets new camera location
			RailCamera->SetActorLocation(NextCameraLocation);
			RailCamera->GetCameraArm()->SetWorldLocation(NextCameraLocationWithOffset);

			// Checks if the camera is in tolerance
			if (NewDistanceAlongSplineWithOffset == this->LastDistanceAlongSplineTarget + RailCamera->CameraSettings.CameraRailOffset
				&& NewDistanceAlongSpline == this->CurrentDistanceAlongSpline)
			{
				bInCameraRailDistanceTolerance = true;
			}
		}
		//In tolerance
		else
		{
// 			LookAtDir = (PlayerActor->GetActorLocation() + CentroidRelativeLocFromPlayer) - RailCamera->GetCameraComponent()->GetComponentLocation();

			float DistanceBetweenCameraAndCurrentInputKey = GetDistanceAlongSplineAtWorldLocation(RailCamera->GetActorLocation()) - GetDistanceAlongSplineAtWorldLocation(NextCameraLocation);

			// Checks if the camera is out tolerance
			if (DistanceBetweenCameraAndCurrentInputKey > RailCamera->CameraSettings.CameraRailDistanceTolerance
				|| DistanceBetweenCameraAndCurrentInputKey < -RailCamera->CameraSettings.CameraRailDistanceTolerance)
			{
				bInCameraRailDistanceTolerance = false;
				NewDistanceAlongSpline = GetDistanceAlongSplineAtWorldLocation(RailCamera->GetActorLocation());
				NewDistanceAlongSplineWithOffset = NewDistanceAlongSpline + RailCamera->CurrentCameraSettings.CameraRailOffset;
				NewCurrentInputKey = FlatSplineComponent->FindInputKeyClosestToWorldLocation(RailCamera->GetActorLocation());
			}
		}

		LookAtDir = (PlayerLoc + CentroidRelativeLocFromPlayer) - RailCamera->GetCameraComponent()->GetComponentLocation();
		FVector LookAtDirRail = (PlayerActor->GetActorLocation() + CentroidRelativeLocFromPlayer) - RailCamera->GetCameraArm()->GetComponentLocation();
		// Computes and sets new camera rotation
		FRotator TargetRotation = FMath::RInterpConstantTo(RailCamera->GetCameraComponent()->GetComponentRotation(), FRotationMatrix::MakeFromX(LookAtDir).Rotator(), DeltaSeconds, RailCamera->CurrentCameraSettings.CameraRotationSpeed);
		FRotator TargetRotationRail = FMath::RInterpConstantTo(RailCamera->GetCameraArm()->GetComponentRotation(), FRotationMatrix::MakeFromX(LookAtDirRail).Rotator(), DeltaSeconds, RailCamera->CurrentCameraSettings.CameraRotationSpeed);
		RailCamera->GetCameraArm()->SetWorldRotation(FRotator::MakeFromEuler(FVector(0.f, 0.f, TargetRotationRail.Yaw)));
		RailCamera->GetCameraComponent()->SetWorldRotation(TargetRotation);
		
		this->CurrentDistanceAlongSpline = NewDistanceAlongSpline;
		this->CurrentDistanceAlongSplineWithOffset = NewDistanceAlongSplineWithOffset;
		this->CurrentInputKey = NewCurrentInputKey;
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

