// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "./Components/CapsuleComponent.h"
#include "InteractableComponent.h"

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
	UInteractableComponent*	GetClosestInteractableObject() { return closestInteractableObject.IsValid() ? closestInteractableObject.Get() : nullptr; }
	UFUNCTION(BlueprintPure)
	UInteractableComponent*	GetCurrentHeldObject() { return holdingObject; }
	UFUNCTION(BlueprintPure)
	EHoldingState			GetCurrentState() { return currentHoldingState; }
	UFUNCTION(BlueprintPure)
	bool	getPushingPoints(FVector& leftPoint, FVector& rightPoint);
public:
	UPROPERTY(EditAnywhere, Category = "Grab")
	FComponentReference	HandleTargetLocationReference;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab")
	float	ThrowPower = 10.0f;
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHoldStateChanged, EHoldingState, previous, EHoldingState, next);
	UPROPERTY(BlueprintAssignable)
	FHoldStateChanged	holdingStateChangedDelegate;

private:
	void	releaseLightGrabbedObject();
	void	createHandConstraint();
	void	releaseHeavyGrabbedObject();

private:
	TWeakObjectPtr<UInteractableComponent>		closestInteractableObject;
	
	UPhysicsHandleComponent*	handleComponent;
	USceneComponent*			handleTargetLocation = nullptr;
	UCapsuleComponent*			characterCapsule = nullptr;

	UInteractableComponent*		holdingObject;
	UPrimitiveComponent*		holdingPrimitiveComponent = nullptr;
	EHoldingState				currentHoldingState = EHoldingState::None;

	UPhysicsConstraintComponent*	leftHandConstraint = nullptr;
	UPhysicsConstraintComponent*	rightHandConstraint = nullptr;
};
