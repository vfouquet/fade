// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/SphereComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractableComponent.generated.h"

class UHoldComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BREATH_API UInteractableComponent : public USceneComponent
{
	GENERATED_BODY()
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FConditionDelegate);

	struct FStickConstraint
	{
		FStickConstraint() = default;
		UPhysicsConstraintComponent*	physicConstraint = nullptr;
		UInteractableComponent*			hook = nullptr;
		UInteractableComponent*			carrier = nullptr;
	};

public:	
	// Sets default values for this component's properties
	UInteractableComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void OnComponentDestroyed(bool destroyedHierarchy) override;

	UFUNCTION(BlueprintCallable)
	void	EraseIdentity();
	UFUNCTION(BlueprintCallable)
	void	GiveIdentity() { identityErased = false; }
	UFUNCTION()
	void	OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit );

	UPhysicsConstraintComponent*	AddStickConstraint(UInteractableComponent* hook, UPrimitiveComponent* stickedObject, FName stickedBoneName);
	
	void							Unstick();
	void							RemoveHookingConstraint(UInteractableComponent* hookToRemove);

	static UInteractableComponent* UInteractableComponent::FindAssociatedInteractableComponent(UPrimitiveComponent* referenceComponent);

	void	SetHoldComponent(UHoldComponent* value = nullptr) { holder = value; }
	void	SetStickingActivated() { isSticked = true; }
	void	SetThrown() { thrown = true; }
	bool	IsSticked() const { return isSticked; }
	UFUNCTION(BlueprintPure)
	bool	HasIdentity() const { return !identityErased; }
	UFUNCTION(BlueprintPure)
	FVector&	GetTempExtent() { return tempExtent; }
	UFUNCTION(BlueprintPure)
	UPrimitiveComponent* GetAssociatedComponent() const { return associatedComponent; }

	bool	IsGrabable() const { return CanBeGrabbed && !identityErased && associatedComponent; }

	//ALL THIS SHIT ARE SECURITY TEST FOR TEMPORARY PUSH/PULL
	UFUNCTION(BlueprintCallable)
	bool	CanRotateLeft(FVector characterForward);
	UFUNCTION(BlueprintCallable)
	bool	CanRotateRight(FVector characterForward);
	UFUNCTION(BlueprintCallable)
	bool	CanPushForward(FVector characterForward);
	UFUNCTION(BlueprintCallable)
	bool	GetDebugLeft();
	UFUNCTION(BlueprintCallable)
	bool	GetDebugCenter();
	UFUNCTION(BlueprintCallable)
	bool	GetDebugRight();
	UFUNCTION(BlueprintCallable)
	bool	GetDebugBack();
	
	FVector							tempExtent; //NON UPDATED EXTENT (CORRECT IF ROTATION IS 0,0,0 AT BEGIN PLAY)
	//

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactions Settings")
	bool	CanBeGrabbed = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactions Settings")
	bool	CanBeSticked = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactions Settings")
	bool	CanAcceptStick = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactions Settings")
	bool	IsHeavy = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactions Settings")
	bool	MemoryInteractable = false;

	UPROPERTY(EditAnywhere, Category = "Interactions Settings")
	FComponentReference	Grab;

	UPROPERTY(BlueprintAssignable)
	FConditionDelegate	onBeginGrab;
	UPROPERTY(BlueprintAssignable)
	FConditionDelegate	onEndGrab;
	UPROPERTY(BlueprintAssignable)
	FConditionDelegate	onBeginStick;
	UPROPERTY(BlueprintAssignable)
	FConditionDelegate	onEndStick;

private:
	TArray<FStickConstraint>		stickingConstraints;
	UPrimitiveComponent*			associatedComponent = nullptr;
	UHoldComponent*					holder = nullptr;

	bool							thrown = false;
	bool							isSticked = false;
	bool							identityErased = false;
};
