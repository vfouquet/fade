// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "./Components/CapsuleComponent.h"
#include "InteractableComponent.h"
#include "MoveComponent.h"

UENUM(BlueprintType)
enum class EHoldingState : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	LightGrabbing UMETA(DisplayName = "LightGrabbing"),
	HeavyGrabbing UMETA(DisplayName = "HeavyGrabbing"),
	Throwing UMETA(DisplayName = "Throwing"),
	Sticking UMETA(DisplayName = "Sticking"),
};
	
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

	void	Action();
	void	Grab();
	void	StopGrab();
	void	Throw();
	void	Stick();

	UFUNCTION(BlueprintPure)
	UInteractableComponent*			GetClosestInteractableObject() const { return closestInteractableObject.IsValid() ? closestInteractableObject.Get() : nullptr; }
	UFUNCTION(BlueprintPure)
	UInteractableComponent*			GetCurrentHeldObject() const { return holdingObject; }
	UFUNCTION(BlueprintPure)
	EHoldingState					GetCurrentState() const { return currentHoldingState; }
	UFUNCTION(BlueprintPure)
	UPhysicsConstraintComponent*	GetLeftHandConstraint() const { return leftHandConstraint; }
	UFUNCTION(BlueprintPure)
	UPhysicsConstraintComponent*	GetRightHandConstraint() const { return rightHandConstraint; }

	UFUNCTION(BlueprintPure)
	FVector	GetHoldingObjectLocation() const { return holdingPrimitiveComponent != nullptr ? holdingPrimitiveComponent->GetComponentLocation() : FVector(); }

	//CURRENTLY PUBLIC FOR BLUEPRINT TESTS
	UFUNCTION(BlueprintPure)
	bool							getPushingPoints(FVector& centerPoint, FVector& leftPoint, FVector& rightPoint) const;
public:
	UPROPERTY(EditAnywhere, Category = "Detection")
	float	DetectionOffset = 30.0f;
	UPROPERTY(EditAnywhere, Category = "Grab")
	FComponentReference	HandleTargetLocationReference;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab")
	float	ThrowPower = 10.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hold")
		float	HoldSnapOffset = 75.0f;
	UPROPERTY(EditAnywhere, Category = "Hold")
	FComponentReference	LeftHandPhysicalConstraintReference;
	UPROPERTY(EditAnywhere, Category = "Hold")
	FComponentReference	RightHandPhysicalConstraintReference;

private:
	void	releaseLightGrabbedObject();
	void	createHandConstraint();
	void	releaseHeavyGrabbedObject();

protected:
	TWeakObjectPtr<UInteractableComponent>		closestInteractableObject;
	
	UPhysicsHandleComponent*		handleComponent;
	UPrimitiveComponent*			handleTargetLocation = nullptr;
	UCapsuleComponent*				characterCapsule = nullptr;

	UPROPERTY(VisibleAnywhere)
	UInteractableComponent*			holdingObject;
	UPrimitiveComponent*			holdingPrimitiveComponent = nullptr;
	EHoldingState					currentHoldingState = EHoldingState::None;

	UPhysicsConstraintComponent*	leftHandConstraint = nullptr;
	UPhysicsConstraintComponent*	rightHandConstraint = nullptr;

	UMoveComponent*					characterMoveComponent = nullptr;
public:
	UPROPERTY(BlueprintAssignable)
	FHoldStateChanged	holdingStateChangedDelegate;
};
