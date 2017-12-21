// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "IdentityEraserComponent.generated.h"


UCLASS(meta=(DisplayName = "IdentityEraser", BlueprintSpawnableComponent) )
class BREATH_API UIdentityEraserComponent : public USphereComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UIdentityEraserComponent();
	virtual void	BeginPlay() override;

	UFUNCTION()
		void	OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	UFUNCTION()
		void	OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
