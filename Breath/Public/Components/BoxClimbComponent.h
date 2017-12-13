// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "BoxClimbComponent.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "ClimbBox", BlueprintSpawnableComponent))
class BREATH_API UBoxClimbComponent : public UBoxComponent
{
	GENERATED_BODY()

public:
	UBoxClimbComponent() { PrimaryComponentTick.bCanEverTick = true; }

	virtual void	BeginPlay() override;
	virtual void	TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	bool	IsOverlappingClimbingSurface() const { return climbingComponents.Num() > 0; }
	bool	IsOverlappingOthers() const;

	bool	CheckSpaceOver() const;
	FVector	GetClimbedLocation() const;

private:
	UFUNCTION()
	void	OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	UFUNCTION()
	void	OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	TArray<TWeakObjectPtr<UPrimitiveComponent>>	climbingComponents;
};
