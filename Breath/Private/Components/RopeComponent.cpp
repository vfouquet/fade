// Fill out your copyright notice in the Description page of Project Settings.

#include "RopeComponent.h"

#include "InteractableComponent.h"

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

	float	tempLength = 0.0f;

	AActor*	owner = GetOwner();
	FVector	ownerLocation = GetOwner()->GetActorLocation();

	spline = NewObject<USplineComponent>(this);
	spline->SetupAttachment(this);
	spline->SetRelativeTransform(FTransform());
	spline->RegisterComponent();

	while (tempLength < Length)
	{
		USphereComponent*	sphere = NewObject<USphereComponent>(this);
		sphere->SetupAttachment(this);
		sphere->SetRelativeTransform(FTransform());
		sphere->RegisterComponent();
		sphere->SetSphereRadius(Thickness * 0.5f, false);
		sphere->SetWorldLocation(ownerLocation + owner->GetActorForwardVector() * tempLength);
		sphere->SetSimulatePhysics(true);
		sphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		sphere->SetCollisionProfileName("SmallInteractable");
		sphere->SetEnableGravity(true);

		tempLength += Thickness;
		spheres.Add(sphere);
		splinePoints.Add(sphere->GetComponentLocation());
	}

	spline->SetSplinePoints(splinePoints, ESplineCoordinateSpace::World, true);
	createSplineMeshes();
	createConstraints();

	if (BeginChild)
	{
		UChildActorComponent* beginChildActorComp = NewObject<UChildActorComponent>(this);
		beginChildActorComp->SetupAttachment(this);
		beginChildActorComp->RegisterComponent();
		beginChildActorComp->SetChildActorClass(BeginChild);
		beginChildActorComp->CreateChildActor();

		beginActor = beginChildActorComp->GetChildActor();
		FVector actorBoundsLoc, actorBoxExtent;
		beginActor->GetActorBounds(true, actorBoundsLoc, actorBoxExtent);

		USceneComponent* componentToAttach = BeginPrimitiveComponentReference.GetComponent(beginActor);
		UPrimitiveComponent* primToAttach = Cast<UPrimitiveComponent>(componentToAttach);
		primToAttach->SetWorldLocation(ownerLocation - owner->GetActorForwardVector() * (Thickness * 0.5f + actorBoxExtent.Size() * 0.5f));

		if (spheres.Num() != 0 && primToAttach)
		{
			UPhysicsConstraintComponent* constraint = NewObject<UPhysicsConstraintComponent>(this);
			constraint->SetupAttachment(this);
			constraint->RegisterComponent();
			constraint->SetWorldLocation(ownerLocation - owner->GetActorForwardVector() * Thickness * 0.5f);
			constraint->SetConstrainedComponents(primToAttach, NAME_None, spheres[0], NAME_None);
			constraints.Add(constraint);
		}
	}

	if (EndChild)
	{
		UChildActorComponent* endChildActorComp = NewObject<UChildActorComponent>(this);
		endChildActorComp->SetupAttachment(this);
		endChildActorComp->RegisterComponent();
		endChildActorComp->SetChildActorClass(EndChild);
		endChildActorComp->CreateChildActor();

		endActor = endChildActorComp->GetChildActor();
		FVector actorBoundsLoc, actorBoxExtent;
		endActor->GetActorBounds(true, actorBoundsLoc, actorBoxExtent);

		USceneComponent* componentToAttach = EndPrimitiveComponentReference.GetComponent(endActor);
		UPrimitiveComponent* primToAttach = Cast<UPrimitiveComponent>(componentToAttach);
		primToAttach->SetWorldLocation(ownerLocation + owner->GetActorForwardVector() * (Length - Thickness * 0.5f + actorBoxExtent.Size() * 0.5f));

		if (spheres.Num() != 0 && primToAttach)
		{
			UPhysicsConstraintComponent* constraint = NewObject<UPhysicsConstraintComponent>(this);
			constraint->SetupAttachment(this);
			constraint->RegisterComponent();
			constraint->SetWorldLocation(ownerLocation + owner->GetActorForwardVector() * (Length - Thickness * 0.5f));
			constraint->SetConstrainedComponents(primToAttach, NAME_None, spheres.Last(), NAME_None);
			constraints.Add(constraint);
		}
	}
}


// Called every frame
void URopeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	updateSplinePoints();
	updateSplineMeshes();
}

void	URopeComponent::createSplineMeshes()
{
	for (int pos = 0; pos < splinePoints.Num() - 1; pos++)
	{
		USplineMeshComponent*	splineMesh = NewObject<USplineMeshComponent>(this);
		//splineMesh->SetupAttachment(this);
		splineMesh->RegisterComponent();
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

void	URopeComponent::createConstraints()
{
	AActor*	owner = GetOwner();
	FVector	ownerLocation = GetOwner()->GetActorLocation();
	for (int pos = 0; pos < splinePoints.Num() - 1; pos++)
	{
		UPhysicsConstraintComponent* constraint = NewObject<UPhysicsConstraintComponent>(this);
		constraint->SetupAttachment(this);
		constraint->RegisterComponent();
		constraint->SetWorldLocation(ownerLocation + owner->GetActorForwardVector() * Thickness * (pos + 1));
		constraint->SetConstrainedComponents(spheres[pos], NAME_None, spheres[pos + 1], NAME_None);
		constraints.Add(constraint);
	}
}

void	URopeComponent::updateSplineMeshes()
{
	for (int pos = 0; pos < splinePoints.Num() - 1; pos++)
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

void	URopeComponent::updateSplinePoints()
{
	splinePoints.Empty(splinePoints.Num());

	for (auto& sphere : spheres)
		splinePoints.Add(sphere->GetComponentLocation());
	spline->SetSplinePoints(splinePoints, ESplineCoordinateSpace::World, true);
}