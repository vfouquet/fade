// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ChemicalWoodComponent.h"
#include "Components/SphereComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "RopeNodeComponent.generated.h"

class USplineMeshComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BREATH_API URopeNodeComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	URopeNodeComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void OnComponentDestroyed(bool destroyedHierarchy) override;

	void					CreateSphere(float size, FVector location);
	UChemicalWoodComponent*	CreateWoodProperty(UPrimitiveComponent* prevPrim, UPrimitiveComponent* nextPrim);
	void					UpdateSplineMesh(FVector beginPoint, FVector beginTangent, FVector endPoint, FVector endTangent);

	void	SetSplineMesh(USplineMeshComponent* value) { splineMesh = value; }
	void	SetPreviousConstraint(UPhysicsConstraintComponent* value) { previousConstraint = value; }
	void	SetNextConstraint(UPhysicsConstraintComponent* value) { nextConstraint = value; }
	void	SetPreviousPrimitive(UPrimitiveComponent* value, bool previousExtremity = false);
	void	SetNextPrimitive(UPrimitiveComponent* value, bool nextExtremity = false);

	USphereComponent*	GetSphere() const { return sphere; }
	FVector				GetSphereLocation() const { return sphere ? sphere->GetComponentLocation() : FVector::ZeroVector; }
	FVector				GetPreviousSplinePointLocation() const;
	FVector				GetNextSplinePointLocation() const;
//private:
	UFUNCTION()
	void	onSphereChemicalStateChanged(EChemicalTransformation transformation, EChemicalState previous, EChemicalState next);

private:
	USplineMeshComponent*						splineMesh = nullptr;
	USphereComponent*							sphere = nullptr;
	TWeakObjectPtr<UPhysicsConstraintComponent>	previousConstraint;
	TWeakObjectPtr<UPhysicsConstraintComponent>	nextConstraint;
	UPrimitiveComponent*						previousPrimitive = nullptr;
	UPrimitiveComponent*						nextPrimitive = nullptr;
	bool										bIsPreviousExtremity = false;
	bool										bIsNextExtremity = false;
	UChemicalWoodComponent*						woodComponent = nullptr;
};
