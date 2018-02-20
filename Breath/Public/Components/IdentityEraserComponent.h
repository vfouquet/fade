// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IdentityZoneComponent.h"
#include "IdentityEraserComponent.generated.h"

class AIdentityZoneManager;

UCLASS(meta=(DisplayName = "IdentityEraser", BlueprintSpawnableComponent) )
class BREATH_API UIdentityEraserComponent : public UIdentityZoneComponent
{
	GENERATED_BODY()

public:
	virtual void	BeginPlay() override;
	virtual void	OnComponentDestroyed(bool bDestroyingHierarchy) override;

	UFUNCTION()
		virtual void	OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult) override;
	UFUNCTION()
		virtual void	OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erased object properties")
		float	DecelerationTime = 0.1f;

	UFUNCTION(BlueprintPure)
		int	GetIndex() const { return erasedIndex; }

private:
	AIdentityZoneManager*	identityZoneManager = nullptr;
	int	erasedIndex = 0;
};
