// Fill out your copyright notice in the Description page of Project Settings.

#include "RopeComponent.h"


// Sets default values for this component's properties
URopeComponent::URopeComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void URopeComponent::BeginPlay()
{
	Super::BeginPlay();

	/*
	float	tempLength = 0.0f;

	AActor*	owner = GetOwner();
	FVector	ownerLocation = GetOwner()->GetActorLocation();

	spline = NewObject<USplineComponent>(this);
	spline->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);
	spline->ClearSplinePoints();

	while (tempLength < Length)
	{
		USphereComponent*	sphere = NewObject<USphereComponent>(this);
		sphere->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);
		sphere->SetSphereRadius(Thickness * 0.5f, false);
		sphere->SetWorldLocation(ownerLocation + owner->GetActorForwardVector() * tempLength);
		sphere->bHiddenInGame = false;
		sphere->SetSimulatePhysics(true);
		sphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		sphere->SetEnableGravity(true);

		FSplinePoint	tempPoint;
		tempPoint.Position = ownerLocation + owner->GetActorForwardVector() * tempLength;
		tempPoint.InputKey = (float)spheres.Num();
		spline->AddPoint(tempPoint, false);
		
		tempLength += Thickness;
		spheres.Add(sphere);
	}
	spline->UpdateSpline();
	createSplineMeshes();
	*/
}


// Called every frame
void URopeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void	URopeComponent::createSplineMeshes()
{
	for (int pos = 0; pos < spheres.Num() - 1; pos++)
	{
		USplineMeshComponent*	splineMesh = NewObject<USplineMeshComponent>(this);
		splineMesh->SetupAttachment(this);
		//splineMesh->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);
		FVector	startPoint;
		FVector	endPoint;
		FVector	startTangent;
		FVector	endTangent;
		spline->GetLocationAndTangentAtSplinePoint(pos, startPoint, startTangent, ESplineCoordinateSpace::World);
		spline->GetLocationAndTangentAtSplinePoint(pos + 1, endPoint, endTangent, ESplineCoordinateSpace::World);
		splineMesh->SetStartAndEnd(startPoint, startTangent, endPoint, endTangent, true);
		splineMeshes.Add(splineMesh);
		splineMesh->SetStaticMesh(mesh);
	}
}

void	URopeComponent::updateSplineMeshes()
{
	for (int pos = 0; pos < spheres.Num() - 1; pos++)
	{
		FVector	startPoint;
		FVector	endPoint;
		FVector	startTangent;
		FVector	endTangent;
		spline->GetLocationAndTangentAtSplinePoint(pos, startPoint, startTangent, ESplineCoordinateSpace::World);
		spline->GetLocationAndTangentAtSplinePoint(pos + 1, endPoint, endTangent, ESplineCoordinateSpace::World);
		splineMeshes[pos]->SetStartAndEnd(startPoint, startTangent, endPoint, endTangent, true);
	}
}