// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "./Components/CapsuleComponent.h"
#include "InteractableComponent.h"

UENUM(BlueprintType)
enum class EHoldingState : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	PreLightGrabbing UMETA(DisplayName = "PreLightGrabbing"),
	LightGrabbing UMETA(DisplayName = "LightGrabbing"),
	PreHeavyGrabbing UMETA(DisplayName = "PreHeavyGrabbing"),
	HeavyGrabbing UMETA(DisplayName = "HeavyGrabbing"),
	Throwing UMETA(DisplayName = "Throwing"),
	HeavyThrowing UMETA(DisplayName = "HeavyThrowing"),
	Sticking UMETA(DisplayName = "Sticking"),
	ReleasingLightGrab UMETA(DisplayName = "ReleasingLightGrab")
};

class AMainCharacter;
	
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HoldComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BREATH_API UHoldComponent : public UActorComponent
{
	GENERATED_BODY()
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHoldStateChanged, EHoldingState, previous, EHoldingState, next);

public:	
	// Sets default values for this component's properties
	UHoldComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

	void	Action();
	void	Grab();
	UFUNCTION(BlueprintCallable)
	void	BeginLightGrabPositionUpdate();
	UFUNCTION(BlueprintCallable)
	void	EndHeavyGrab();
	void	EndLightGrabRelease();
	void	StopGrab();
	void	Throw();
	UFUNCTION(BlueprintCallable)
	void	EndThrow();
	void	Stick();
	void	UniversalRelease();
	void	CancelThrow();
	void	CancelHeavyThrow();
	void	CancelLightGrab();
	void	CancelHeavyGrab();

	UFUNCTION(BlueprintPure)
	UInteractableComponent*			GetClosestInteractableObject() const { return closestInteractableObject.IsValid() ? closestInteractableObject.Get() : nullptr; }
	UFUNCTION(BlueprintPure)
	UInteractableComponent*			GetCurrentHeldObject() const { return holdingObject.Get(); }
	UFUNCTION(BlueprintPure)
	EHoldingState					GetCurrentState() const { return currentHoldingState; }
	UFUNCTION(BlueprintPure)
	UPhysicsConstraintComponent*	GetLeftHandConstraint() const { return leftHandConstraint; }
	UFUNCTION(BlueprintPure)
	UPhysicsConstraintComponent*	GetRightHandConstraint() const { return rightHandConstraint; }

	bool	CanRotateLeft(FVector characterForward) { return holdingObject.IsValid() && holdingObject->CanRotateLeft(characterForward); }
	bool	CanRotateRight(FVector characterForward) { return holdingObject.IsValid() && holdingObject->CanRotateRight(characterForward); }
	bool	CanPushForward(FVector characterForward) { return holdingObject.IsValid() && holdingObject->CanPushForward(characterForward); }

	bool	IsMovingHeavyObject() const { return currentHoldingState == EHoldingState::HeavyGrabbing; }
	bool	IsHoldingObject() const { return currentHoldingState == EHoldingState::LightGrabbing; }

	UFUNCTION(BlueprintPure)
	FVector	GetHoldingObjectLocation() const { return holdingPrimitiveComponent != nullptr ? holdingPrimitiveComponent->GetComponentLocation() : FVector(); }
	UFUNCTION(BlueprintPure)
	float	GetHoldingObjectMass() const { return holdingPrimitiveComponent != nullptr ? holdingPrimitiveComponent->GetMass() : 0.0f; }

	//CURRENTLY PUBLIC FOR BLUEPRINT TESTS
	UFUNCTION(BlueprintPure)
	bool							getPushingPoints(FVector& centerPoint, FVector& leftPoint, FVector& rightPoint) const;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
	float	DetectionOffset = 60.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
	float	HoldingDetectionOffset = 100.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab")
	float	ThrowPower = 10.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab")
	float	HeavyThrowPower = 10.0f;
	UPROPERTY(EditAnywhere, Category = "Hold")
	FComponentReference	LeftHandPhysicalConstraintReference;
	UPROPERTY(EditAnywhere, Category = "Hold")
	FComponentReference	RightHandPhysicalConstraintReference;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Throw")
	float	AdditionalThrowAngle = 45.0f;

private:
	void	releaseLightGrabbedObject();
	void	createHandConstraint();
	void	releaseHeavyGrabbedObject();

	void	detectInteractableAround();
protected:
	AMainCharacter*								mainCharacter = nullptr;
	TWeakObjectPtr<UInteractableComponent>		closestInteractableObject;
	FVector										closestInteractableNormal;
	FVector										closestInteractableLocation;

	UCapsuleComponent*				characterCapsule = nullptr;
	bool							previousGravityValue = false;

	UPROPERTY(VisibleAnywhere)
	TWeakObjectPtr<UInteractableComponent>	holdingObject;
	UPrimitiveComponent*					holdingPrimitiveComponent = nullptr;
	EHoldingState							currentHoldingState = EHoldingState::None;

	float	releaseCurrentTime = 0.0f;

	UPhysicsConstraintComponent*	leftHandConstraint = nullptr;
	UPhysicsConstraintComponent*	rightHandConstraint = nullptr;
public:
	UPROPERTY(BlueprintAssignable)
	FHoldStateChanged	holdingStateChangedDelegate;
};
