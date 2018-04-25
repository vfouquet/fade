// Fill out your copyright notice in the Description page of Project Settings.

#include "CloudGenerator.h"

#include "Engine/World.h"
#include "Components/BoxComponent.h"

// Sets default values
ACloudGenerator::ACloudGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	
}

// Called when the game starts or when spawned
void ACloudGenerator::BeginPlay()
{
	Super::BeginPlay();
	
	if (CloudSamples.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s : No samples added"), *GetName());
		return;
	}

	UBoxComponent* box = NewObject<UBoxComponent>(this);
	if (!box)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s : Couldn't create cloud box"), *GetName());
		return;
	}
	
	box->SetupAttachment(GetRootComponent());
	box->RegisterComponent();
	box->SetCollisionProfileName("Cloud");
	box->SetBoxExtent(CloudZoneSize);

	FVector	actorLocation = GetActorLocation();
	float	baseHeight = actorLocation.Z - HeightSlotsNumber * CloudZoneSize.Z * 0.25f;

	for (int slotNbr = 0; slotNbr < HeightSlotsNumber; slotNbr++)
	{
		for (int cloudNbr = 0; cloudNbr < SlotCloudsNumber; cloudNbr++)
		{
			FVector location = actorLocation;
			location.X += FMath::FRandRange(-CloudZoneSize.X, CloudZoneSize.X);
			location.Y += FMath::FRandRange(-CloudZoneSize.Y, CloudZoneSize.Y);
			location.Z = baseHeight;
			generateCloud(location);
		}
		baseHeight += CloudZoneSize.Z;
	}
}

// Called every frame
void ACloudGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	for (auto& cloud : clouds)
	{
		if (!cloud.Key.IsValid())
		{
			clouds.Remove(cloud.Key);
			continue;
		}
		cloud.Key->AddActorWorldOffset(cloud.Key->GetActorForwardVector() * cloud.Value);
	}
}

void	ACloudGenerator::generateCloud(FVector location)
{
	FTransform	spawnTransform;
	spawnTransform.SetLocation(location);
	spawnTransform.SetRotation(FRotator(0.0f, FMath::FRandRange(0.0f, 360.0f), 0.0f).Quaternion());
	FActorSpawnParameters	parameters;

	AActor* cloud = GetWorld()->SpawnActor(CloudSamples[FMath::RandRange(0, CloudSamples.Num() - 1)], &spawnTransform, parameters);
	cloud->SetActorScale3D(FVector(FMath::FRandRange(CloudSizeRange.X, CloudSizeRange.Y)));
	clouds.Add(cloud, FMath::FRandRange(CloudSpeedRange.X, CloudSpeedRange.Y));
}