// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Components/ActorComponent.h"
#include "CharacterHealthComponent.generated.h"

UENUM(BlueprintType)
enum class ECharacterCondition : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Scalding = 1 UMETA(DisplayName = "Scalding"),
	Burning = 2 UMETA(DisplayName = "Burning")
};

UENUM(BlueprintType)
enum class ECharacterDamageState : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Wounded = 1 UMETA(DisplayName = "Wounded"),
	Dead = 2 UMETA(DisplayName = "Dead")
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BREATH_API UCharacterHealthComponent : public UActorComponent
{
	GENERATED_BODY()
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCharacterConditionDelegate, ECharacterCondition, previousCondition, ECharacterCondition, nextCondition);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCharacterDamageStateDelegate, ECharacterDamageState, previousDamageState, ECharacterDamageState, nextDamageState);

public:
	// Sets default values for this component's properties
	UCharacterHealthComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION()
		void	OnCapsuleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	UFUNCTION()
		void	OnCapsuleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	UFUNCTION()
		void	OnCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION(BlueprintPure)
	ECharacterCondition	const& getCurrentCondition() const { return currentCondition; }
	UFUNCTION(BlueprintPure)
	ECharacterDamageState const& getCurrentDamageState() const { return currentDamageState; }
	
	UFUNCTION(BlueprintPure)
	float const getCurrentFireTime() const { return currentFireTime; }
	UFUNCTION(BlueprintPure)
	float const getCurrentDamageTime() const { return currentDamageTime; }
	UFUNCTION(BlueprintPure)
	int	const getCurrentFireZoneCount() const { return fireCount; }
	UFUNCTION(BlueprintPure)
	int const getCurrentEraserZoneCount() const { return eraseZoneCount; }
	UFUNCTION(BlueprintPure)
	int const getCurrentMemoryZoneCount() const { return memoryZoneCount; }
	UFUNCTION(BlueprintPure)
	bool const isAffectedByFire() const { return eraseZoneCount == 0 || (eraseZoneCount > 0 && memoryZoneCount > 0); }

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire")
	float	ScaldingToBurning = 3.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire")
	float	BurningToDeath = 5.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
	float	ImpactForce2Threshold = 25000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
	float	ImpactForce3Threshold = 50000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
	float	ImpactMeshForce = 1000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
	float	WoundedResetTime = 3.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impact")
	float	FatalJumpHeight = 500.0f;
	UPROPERTY(BlueprintAssignable)
	FCharacterConditionDelegate	onConditionChanged;
	UPROPERTY(BlueprintAssignable)
	FCharacterDamageStateDelegate		onDamageStateChanged;

private:
	void	takeFireDamage();
	void	decreaseFireCount();
	void	applyWaterEffect();
	void	die(FVector	impact = FVector::ZeroVector, FVector impactLocation = FVector::ZeroVector, FName boneName = NAME_None);

private:
	ECharacterCondition		currentCondition = ECharacterCondition::None;
	ECharacterDamageState	currentDamageState = ECharacterDamageState::None;

	float	currentFireTime = 0.0f;
	float	currentDamageTime = 0.0f;

	float	jumpZOffset = 0.0f;

	int	fireCount = 0;
	int	eraseZoneCount = 0;
	int	memoryZoneCount = 0;
};
