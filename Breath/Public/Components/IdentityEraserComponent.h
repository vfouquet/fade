// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IdentityZoneComponent.h"
#include "IdentityEraserComponent.generated.h"

UCLASS(meta=(DisplayName = "IdentityEraser", BlueprintSpawnableComponent) )
class BREATH_API UIdentityEraserComponent : public UIdentityZoneComponent
{
	GENERATED_BODY()

public:	
	UFUNCTION()
	virtual void	OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult) override;
	UFUNCTION()
	virtual void	OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erased object properties")
	float	DecelerationTime = 0.1f;
};
