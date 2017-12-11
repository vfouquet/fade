// Fill out your copyright notice in the Description page of Project Settings.

#include "CameraRailPoint.h"

#include "Editor/UnrealEd/Public/Editor.h"
#include "Kismet/GameplayStatics.h"

#include "Cameras/CameraRailManager.h"




// Sets default values
ACameraRailPoint::ACameraRailPoint()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	SetRootComponent(RootComponent);

	SplinePoint.CameraRailPoint = this;
	SplinePoint.Position = GetActorLocation();
	SplinePoint.Rotation = GetActorRotation();
}

void ACameraRailPoint::AddPreviousPointImpl()
{
#if WITH_EDITOR
	GEditor->BeginTransaction(FText::FromString("Add Previous Point"));

	ACameraRailPoint* NewPoint = GetWorld()->SpawnActor<ACameraRailPoint>(GetClass(), GetTransform());
	NewPoint->SplinePoint.Position = GetActorLocation();
	NewPoint->SplinePoint.Rotation = GetActorRotation();
	NewPoint->InCameraSettings = this->InCameraSettings;
	NewPoint->OutCameraSettings = this->OutCameraSettings;

	if (PreviousPoint.IsValid())
	{
		PreviousPoint->NextPoint = NewPoint;
	}

	NewPoint->NextPoint = this;
	NewPoint->PreviousPoint = PreviousPoint;
	NewPoint->Modify();
	PreviousPoint = NewPoint;
	PreviousPoint->Modify();
	GEditor->SelectNone(true, true);
	GEditor->SelectActor(NewPoint, true, true, true, true);

	this->Modify();
	GEditor->EndTransaction();

	TArray<AActor*>	FoundActors;

	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACameraRailManager::StaticClass(), FoundActors);

	if (FoundActors.Num() > 0)
	{
		ACameraRailManager*	CameraRailManager = Cast<ACameraRailManager>(FoundActors[0]);
		CameraRailManager->UpdateSpline();
	}
#endif
}

void ACameraRailPoint::AddNextPointImpl()
{
#if WITH_EDITOR
	GEditor->BeginTransaction(FText::FromString("Add Next Point"));

	ACameraRailPoint* NewPoint = GetWorld()->SpawnActor<ACameraRailPoint>(GetClass(), GetTransform());
	NewPoint->SplinePoint.Position = GetActorLocation();
	NewPoint->SplinePoint.Rotation = GetActorRotation();
	NewPoint->InCameraSettings = this->InCameraSettings;
	NewPoint->OutCameraSettings = this->OutCameraSettings;

	if (NextPoint.IsValid())
	{
		NextPoint->PreviousPoint = NewPoint;
	}

	NewPoint->PreviousPoint = this;
	NewPoint->NextPoint = NextPoint;
	NewPoint->Modify();
	NextPoint = NewPoint;
	NextPoint->Modify();
	GEditor->SelectNone(true, true);
	GEditor->SelectActor(NewPoint, true, true, true, true);

	this->Modify();
	GEditor->EndTransaction();

	if (CurrentCameraRailManager != nullptr)
	{
		CurrentCameraRailManager->UpdateSpline();
	}
#endif
}

void ACameraRailPoint::RemoveCurrentPointImpl()
{
#if WITH_EDITOR
	GEditor->BeginTransaction(FText::FromString("Remove Point"));
	if (PreviousPoint.IsValid())
	{
		 PreviousPoint->NextPoint = NextPoint;
		 PreviousPoint->Modify();
	}
	if (NextPoint.IsValid())
	{
		NextPoint->PreviousPoint = PreviousPoint;
		NextPoint->Modify();
	}

	this->Destroy();
	GEditor->EndTransaction();

	if (CurrentCameraRailManager != nullptr)
	{
		CurrentCameraRailManager->UpdateSpline();
	}
#endif
}

// Called when the game starts or when spawned
void ACameraRailPoint::BeginPlay()
{
	Super::BeginPlay();
}

void ACameraRailPoint::PreEditChange(UProperty* PropertyThatWillChange)
{
	Super::PreEditChange(PropertyThatWillChange);

#if WITH_EDITOR
	if (PropertyThatWillChange != nullptr)
	{
		if (PropertyThatWillChange->GetNameCPP() == "PreviousPoint" && this->PreviousPoint.IsValid())
		{
			GEditor->BeginTransaction(FText::FromString("Modify Camera Point"));
			this->PreviousPoint->NextPoint = nullptr;
			this->PreviousPoint->Modify();
			this->Modify();
			GEditor->EndTransaction();
		}
		else if (PropertyThatWillChange->GetNameCPP() == "NextPoint" && this->NextPoint.IsValid())
		{
			GEditor->BeginTransaction(FText::FromString("Modify Camera Point"));
			this->NextPoint->PreviousPoint = nullptr;
			this->NextPoint->Modify();
			this->Modify();
			GEditor->EndTransaction();
		}
	}
#endif
}

void ACameraRailPoint::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

#if WITH_EDITOR
	if (PropertyChangedEvent.ChangeType == EPropertyChangeType::ValueSet)
	{
		if (PropertyChangedEvent.GetPropertyName() == "PreviousPoint" && this->PreviousPoint.IsValid())
		{
			GEditor->BeginTransaction(FText::FromString("Modify Camera Point"));
			if (this->PreviousPoint->NextPoint.IsValid() && this->PreviousPoint->NextPoint != this)
			{
				if (this->PreviousPoint->NextPoint->PreviousPoint.IsValid())
				{
					this->PreviousPoint->NextPoint->PreviousPoint = nullptr;
					this->PreviousPoint->NextPoint->Modify();
				}
				this->PreviousPoint->NextPoint = nullptr;
				this->PreviousPoint->Modify();
			}

			this->PreviousPoint->NextPoint = this;
			this->Modify();
			GEditor->EndTransaction();
		}
		else if (PropertyChangedEvent.GetPropertyName() == "NextPoint" && this->NextPoint.IsValid())
		{
			GEditor->BeginTransaction(FText::FromString("Modify Camera Point"));
			if (this->NextPoint->PreviousPoint.IsValid() && this->NextPoint->PreviousPoint != this)
			{
				if (this->NextPoint->PreviousPoint->NextPoint.IsValid())
				{
					this->NextPoint->PreviousPoint->NextPoint = nullptr;
					this->NextPoint->PreviousPoint->Modify();
				}
				this->NextPoint->PreviousPoint = nullptr;
				this->NextPoint->Modify();
			}

			this->NextPoint->PreviousPoint = this;
			this->Modify();
			GEditor->EndTransaction();
		}
	}

	if (CurrentCameraRailManager != nullptr)
	{
		CurrentCameraRailManager->UpdateSpline();
	}
#endif
}

void ACameraRailPoint::PostEditMove(bool bFinished)
{
	Super::PostEditMove(bFinished);

#if WITH_EDITOR
	SplinePoint.Position = GetActorLocation();
	SplinePoint.Rotation = GetActorRotation();

	if (CurrentCameraRailManager != nullptr)
	{
		CurrentCameraRailManager->UpdateSpline();
	}
#endif
}

void ACameraRailPoint::Destroyed()
{
	Super::Destroyed();

	RemoveCurrentPointImpl();
}


// Called every frame
void ACameraRailPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
