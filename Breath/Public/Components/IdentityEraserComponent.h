// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AkAudio/Classes/AkAudioEvent.h"

#include "CoreMinimal.h"
#include "IdentityZoneComponent.h"
#include "IdentityEraserComponent.generated.h"

class UParticleSystemComponent;

UCLASS(meta=(DisplayName = "IdentityEraser", BlueprintSpawnableComponent) )
class BREATH_API UIdentityEraserComponent : public UIdentityZoneComponent
{
	GENERATED_BODY()

public:
	virtual void	BeginPlay() override;
	virtual void	TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void	OnComponentDestroyed(bool bDestroyingHierarchy) override;

	UFUNCTION()
	virtual void	OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult) override;
	UFUNCTION()
	virtual void	OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;

	UFUNCTION(BlueprintCallable)
	void	SetCinematicPlayed() { cinematicPlayed = true; }
	UFUNCTION(BlueprintPure)
	bool	HasCinematicBeenPlayed() const { return cinematicPlayed; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Erased object properties")
	float	DecelerationTime = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound properties")
	UAkAudioEvent*	AudioCinematic = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	bool	UseVisualEffects = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle properties")
	UParticleSystem*	WaveParticleTemplate = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume Effect")
	UStaticMesh*		SphereMesh = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume Effect")
	UMaterialInterface*		SphereOpaqueMaterial = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volume Effect")
	UMaterialInterface*		SphereTransparentMaterial = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shader")
	float	FallOffValue = 0.0f;

private:
	UParticleSystemComponent*	particleSystem = nullptr;
	bool						cinematicPlayed = false;
};
