// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CloudGenerator.generated.h"

UCLASS()
class BREATH_API ACloudGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACloudGenerator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud")
	TArray<UClass*>	CloudSamples;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud")
	int	HeightSlotsNumber = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud")
	FVector2D	CloudSizeRange;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud")
	FVector2D	CloudSpeedRange;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud")
	FVector	CloudZoneSize;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud")
	int	SlotCloudsNumber = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud")
	FRotator	CloudDirection;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud")
	bool	bOverrideCloudRotation = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud")
	FVector2D	CloudRotationRange;

private:
	void	generateCloud(FVector location);

private:
	TMap<TWeakObjectPtr<AActor>, float>	clouds;
};
