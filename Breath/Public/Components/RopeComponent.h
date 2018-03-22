// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/SphereComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"

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
	UStaticMesh*	mesh = nullptr;
	UPROPERTY(EditAnywhere, Category = "Rope")
	FComponentReference	BeginComponentReference;
	UPROPERTY(EditAnywhere, Category = "Rope")
	FComponentReference	EndComponentReference;

	UPROPERTY(EditAnywhere, Category = "Stick options")
	FComponentReference	BeginComponentStickOverride;
	UPROPERTY(EditAnywhere, Category = "Stick options")
	FComponentReference	EndComponentStickOverride;

private:
	void	createSplineMeshes();
	void	createConstraints();
	void	updateSplineMeshes();
	void	updateSplinePoints();

private:
	TArray<USphereComponent*>				spheres;
	TArray<USplineMeshComponent*>			splineMeshes;
	TArray<FVector>							splinePoints;
	TArray<UPhysicsConstraintComponent*>	constraints;
	USplineComponent*						spline = nullptr;
	UPrimitiveComponent*					beginAttachPrimitive = nullptr;
	UPrimitiveComponent*					endAttachPrimitive = nullptr;
	bool									isInit = false;
};
