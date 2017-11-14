// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PhysicsEngine/PhysicsConstraintComponent.h"

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractableComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BREATH_API UInteractableComponent : public UActorComponent
{
	GENERATED_BODY()

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

	UPhysicsConstraintComponent*	AddStickConstraint(UInteractableComponent* hook, UPrimitiveComponent* stickedObject, FName stickedBoneName);
	void							Unstick();
	void							RemoveHookingConstraint(UInteractableComponent* hookToRemove);

	void	SetStickingActivated() { isSticked = true; }
	bool	IsSticked() const { return isSticked; }
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactions Settings")
	bool	CanBeGrabbed = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactions Settings")
	bool	CanBeSticked = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactions Settings")
	bool	CanAcceptStick = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interactions Settings")
	bool	IsHeavy = false;

private:
	TArray<FStickConstraint>	stickingConstraints;

	bool						isSticked = false;
};
