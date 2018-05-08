// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Array.h"
#include "MainCharacterMovementComponent.h"
#include "AkAudio/Classes/AkAudioEvent.h"
#include "Map.h"
#include "TimerManager.h"

UENUM(BlueprintType)
enum class EClimbType : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	OneMeterClimb = 1 UMETA(DisplayName = "OneMeterClimb"),
	TwoMetersClimb = 2 UMETA(DisplayName = "TwoMetersClimb"),
	AirOneMeterClimb = 3 UMETA(DisplayName = "AirOneMeterClimb"),
	AirTwoMetersClimb = 4 UMETA(DisplayName = "AirTwoMetersClimb")
};

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
};

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MainCharacter.generated.h"

class USpringArmComponent;
class UPlayerCameraComponent;
class UHoldComponent;
class AMainPlayerController;

UCLASS()
class BREATH_API AMainCharacter : public ACharacter
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDeathDelegate);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDamageDelegate, FVector, impactDir);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FIdentityZoneDelegate, UIdentityEraserComponent*, Zone);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCharacterConditionDelegate, ECharacterCondition, previousCondition, ECharacterCondition, nextCondition);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCharacterDamageStateDelegate, ECharacterDamageState, previousDamageState, ECharacterDamageState, nextDamageState);
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool	bIsThirdPersonCamera;

public:
	// Sets default values for this character's properties
	AMainCharacter(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void	Move(FVector InputVector);

	void	RotateHorizontal(float Value);
	void	RotateVertical(float Value);

	void	Action();
	void	BeginGrab();
	UFUNCTION(BlueprintCallable)
	void	StopGrab();
	void	Throw();
	void	Stick();
	void	Jump() override;
	bool	Climb();
	
	void	SnapCharacterTo(FVector location, FRotator rotation, float time, FTimerDelegate& del);
	void	StopSnapping();
	UFUNCTION(BlueprintCallable)
	void	EndThrow();
	UFUNCTION(BlueprintCallable)
	void	BeginGrabPositionUpdate();
	UFUNCTION(BlueprintCallable)
	void	EndReleaseLightGrab();
	UFUNCTION(BlueprintCallable)
	void	DealDamage(FHitResult hitResult, UPrimitiveComponent* damageDealer, bool HeavyDamage = true);
	void	OnDamage(FVector impactDir = FVector::ZeroVector);
	void	Die(FVector impact = FVector::ZeroVector, FVector impactLoc = FVector::ZeroVector, FName boneName = NAME_None);
	UFUNCTION()
	void	OnCapsuleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	UFUNCTION()
	void	OnCapsuleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable)
	void	HeadLookAt(FVector lookAtLocation);

	UFUNCTION(BlueprintCallable)
	void	SetWalkingInDeepWater(bool const value);
	UFUNCTION(BlueprintCallable)
	void	SetCautionWalkMode(bool const value) { bIsCautionWalking = value; }
	void	SetWalkMode();
	void	SetJogMode();
	UFUNCTION(BlueprintCallable)
	void	SetHeadRotation(FRotator value);
	UFUNCTION(BlueprintCallable)
	void	SetCustomSpeed(bool customSpeed, float newSpeed = 0.0f);
	UFUNCTION(BlueprintCallable)
	void	UnsetCustomSpeed();

	void	SetRotatingLeft(bool const value) { rotatingLeft = value; }
	void	SetRotatingRight(bool const value) { rotatingRight = value; }
	void	SetPushingAxis(float const& value) { pushingAxis = value; }	
	void	SetCanAutoClimb(bool const value) { bCanAutoClimbInAir = value; }

	UFUNCTION(BlueprintCallable)
	void	BlockCharacterJump() { bJumpLocked = true; }
	UFUNCTION(BlueprintCallable)
	void	UnblockCharacterJump() { bJumpLocked = false; }
	UFUNCTION(BlueprintCallable)
	void	BlockCharacter() { bBlocked = true; }
	UFUNCTION(BlueprintCallable)
	void	UnblockCharacter() { bBlocked = false; }
	void	EnableMovingHeavyObjectMode();
	void	DisableMovingHeavyObjectMode();
	void	SetThrowingObject(bool const value) { bThrowingObject = value; }
	void	SetHoldingObject(bool const value) { bHoldingObject = value; }

	void	PlayLightGrabMontage(bool oneMeter = false);
	void	PlayLightGrabReleaseMontage();
	void	PlayLightThrowMontage();
	UFUNCTION()
	void	PlayHeavyGrabMontage();
	void	PlayHeavyThrowMontage();
	void	StopLightGrabMontage() { StopAnimMontage(LightGrabAnim); }
	void	StopHeavyGrabMontage() { StopAnimMontage(HeavyGrabAnim); }

	void	PlayMeteorEvent();
	void	StopMeteorEvent();

	UFUNCTION(BlueprintPure)
	bool			IsWalkingInDeepWater() const { return bIsWalkingInDeepWater; }
	UFUNCTION(BlueprintPure)
	bool			IsCautionwalking() const { return bIsCautionWalking; }
	UFUNCTION(BlueprintPure)
	bool			IsLongJumping() const { return bIsLongJump; }
	UFUNCTION(BlueprintPure)
	bool			IsClimbing() const { return isClimbing; }
	UFUNCTION(BlueprintPure)
	bool			IsHeavyModeEnabled() const { return bMovingHeavyObject; }
	UFUNCTION(BlueprintPure)
	bool			IsRotatingLeft() const { return rotatingLeft; }
	UFUNCTION(BlueprintPure)
	bool			IsRotatingRight() const { return rotatingRight; }
	UFUNCTION(BlueprintPure)
	bool			CanThrow() const;
	UFUNCTION(BlueprintPure)
	bool			IsHoldingObject() const { return bHoldingObject; }
	bool			IsInAir() const;
	UFUNCTION(BlueprintPure)
	bool			isDead() const { return bIsDead; }
	UFUNCTION(BlueprintPure)
	FRotator const	GetHeadRotation() const { return headRotation; }
	UFUNCTION(BlueprintPure)
	FVector			GetTwoHandsLocation() const;
	UFUNCTION(BlueprintPure)
	AActor*			GetHeldActor();
	UFUNCTION(BlueprintPure)
	float			GetPushAndPullCoef() const { return pushingAxis; }

	UFUNCTION(BlueprintPure)
	ECharacterCondition	const& GetCurrentCondition() const { return currentCondition; }
	UFUNCTION(BlueprintPure)
	ECharacterDamageState const& GetCurrentDamageState() const { return currentDamageState; }
	UFUNCTION(BlueprintPure)
	bool IsGod() const { return bIsGod; }
	UFUNCTION(BlueprintPure)
	float const GetCurrentFireTime() const { return currentFireTime; }
	UFUNCTION(BlueprintPure)
	float const GetCurrentDamageTime() const { return currentDamageTime; }
	UFUNCTION(BlueprintPure)
	int	const GetCurrentFireZoneCount() const { return fireCount; }
	UFUNCTION(BlueprintPure)
	bool IsErased() const { return memoryZoneCount == 0 && eraseZoneCount > 0; }
	UFUNCTION(BlueprintPure)
	int const GetCurrentEraserZoneCount() const { return eraseZoneCount; }
	UFUNCTION(BlueprintPure)
	int const GetCurrentMemoryZoneCount() const { return memoryZoneCount; }
	UFUNCTION(BlueprintPure)
	bool const IsAffectedByFire() const { return eraseZoneCount == 0 || (eraseZoneCount > 0 && memoryZoneCount > 0); }
	UFUNCTION(BlueprintCallable)
	void	SetGodMode(bool value) { bIsGod = value; }

public:
	/*Roll isn't used*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Head")
	FRotator	MaxHeadRotationRange;
	/*Time to validate climb by walking**/
	UPROPERTY(EditAnywhere, Category = "Climb")
	float	ClimbAngleTolerence = 45.0f;
	UPROPERTY(EditAnywhere, Category = "Climb")
	float	ClimbSnapTime = 0.1f;

	UPROPERTY(BlueprintAssignable)
	FDeathDelegate	OnDie;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump")
	float	MinLongJumpVelocity = 250.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimMontage*	Climb1MeterMontage = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimMontage*	Climb2MetersMontage = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimMontage*	LightGrabAnim = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimMontage*	LightGrabReleaseAnim = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimMontage*	LightGrabOneMeterAnim = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimMontage*	LightThrowAnim = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimMontage*	HeavyGrabAnim = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimMontage*	HeavyPushAnim = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
	UAnimMontage*	JumpReceptionAnim = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	UAkAudioEvent*	MeteorEvent = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	UAkAudioEvent*	StoppingMeteorEvent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float	ScaldingToBurning = 3.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float	BurningToDeath = 5.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float	WoundedResetTime = 3.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float	FatalJumpHeight = 500.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float	ReceptionJumpHeight = 250.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float	ImpactMeshForce = 1000.0f;
	UPROPERTY(BlueprintAssignable)
	FCharacterConditionDelegate		onConditionChanged;
	UPROPERTY(BlueprintAssignable)
	FCharacterDamageStateDelegate	onDamageStateChanged;
	UPROPERTY(BlueprintAssignable)
	FIdentityZoneDelegate			onCharacterErased;
	UPROPERTY(BlueprintAssignable)
	FIdentityZoneDelegate			onCharacterGetIdentity;
	UPROPERTY(BlueprintAssignable)
	FDamageDelegate					onCharacterDamage;

private:
	EClimbType	climbTrace(FVector& outHitLocation, FVector& outNormal, FVector& outTopPoint);
	UFUNCTION()
	void	onEndMontage(UAnimMontage* montage, bool bInterrupted);
	UFUNCTION()
	void	endCharacterClimbSnap();
	UFUNCTION()
	void	endClimb();
	void	stopCurrentPlayingMontage() { StopAnimMontage(); }
	void	takeFireDamage();
	void	decreaseFireCount();
	void	applyWaterEffect();
	void	updateHealthValues(float DelatTime);
	bool	isAffectedByFire() const { return eraseZoneCount == 0 || (eraseZoneCount > 0 && memoryZoneCount > 0); }

private:
	UPROPERTY(VisibleAnywhere)
	USpringArmComponent*				SpringArmComponent;
	UPlayerCameraComponent*				CameraComponent;
	UHoldComponent*						holdComponent = nullptr;  //REMOVE THIS WITH PUSH/PULL TEMP CALL
	UMainCharacterMovementComponent*	mainCharacterMovement = nullptr;

	FTimerHandle		climbSnapTimerHandle;

	FRotator			headRotation;

	bool	bCustomSpeedEnabled = false;
	bool	bBlocked = false;
	bool	bJumpLocked = false;
	bool	bMovingHeavyObject = false;
	bool	bThrowingObject = false;
	bool	bHoldingObject = false;
	bool	bIsDead = false;
	
	//PUSH/PULL INPUT FROM CONTROLLER
	bool	rotatingLeft = false;
	bool	rotatingRight = false;
	float	pushingAxis = 0.0f;
	//CLIMBINPUT FROM CONTROLLER
	bool	bCanAutoClimbInAir = false;

	bool	bIsLongJump = false;
	bool	bIsCautionWalking = false;
	bool	bIsWalkingInDeepWater = false;

	//TEMP CLIMB TRICK SHIT
	EClimbType	climbType;
	bool	isClimbing = false;
	FVector	beginClimbActorLocation;
	FVector	hipBeginLocation;
	float	zClimbTarget = 0.0f;

	//HEALTH SETTINGS
	TMap<TWeakObjectPtr<UPrimitiveComponent>, float>	damageDealers;

	ECharacterCondition		currentCondition = ECharacterCondition::None;
	ECharacterDamageState	currentDamageState = ECharacterDamageState::None;

	float	currentFireTime = 0.0f;
	float	currentDamageTime = 0.0f;

	float	jumpZOffset = 0.0f;

	int		fireCount = 0;
	int		eraseZoneCount = 0;
	int		memoryZoneCount = 0;
	bool	bIsGod = false;
};
