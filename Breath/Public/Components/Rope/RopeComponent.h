// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ChemicalWoodComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "RopeNodeComponent.h"

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "RopeComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BREATH_API URopeComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	URopeComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rope")
	bool	CanBurn = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rope")
	float	Thickness = 10.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rope")
	float	ExtraLength = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rope")
	UStaticMesh*	mesh = nullptr;
	UPROPERTY(EditAnywhere, Category = "Rope")
	FComponentReference	BeginComponentReference;
	UPROPERTY(EditAnywhere, Category = "Rope")
	FComponentReference	EndComponentReference;

	UPROPERTY(EditAnywhere, Category = "Stick options")
	FComponentReference	BeginComponentStickOverride;
	UPROPERTY(EditAnywhere, Category = "Stick options")
	FComponentReference	EndComponentStickOverride;

	UFUNCTION(BlueprintCallable)
	void	DestroyNode(int idx) { if (idx < nodes.Num()) nodes[idx]->onSphereChemicalStateChanged(EChemicalTransformation::Burning, EChemicalState::None, EChemicalState::Scorched); }

private:
	void	createSplineMeshes();
	void	createConstraints();
	void	updateSplineMeshes();

	void	attachBeginPrimitive();
	void	attachEndPrimitive();
	void	destroyRope();

private:
	TArray<TWeakObjectPtr<URopeNodeComponent>>	nodes;
	USplineComponent*							spline = nullptr;
	UPrimitiveComponent*						beginAttachPrimitive = nullptr;
	UPrimitiveComponent*						endAttachPrimitive = nullptr;
	bool										isInit = false;
};
