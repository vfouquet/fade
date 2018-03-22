// Fill out your copyright notice in the Description page of Project Settings.

#include "RopeComponent.h"

#include "InteractableComponent.h"
#include "RopeAttachmentComponent.h"

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
	UChildActorComponent*	beginComponent = Cast<UChildActorComponent>(BeginComponentReference.GetComponent(owner));
	UChildActorComponent*	endComponent = Cast<UChildActorComponent>(EndComponentReference.GetComponent(owner));
	if (!beginComponent || !endComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s-Rope Component : BeginChildActorComponent or EndChildActorComponent is wrong"), owner?*owner->GetName() : *FString("Error"));
		return;
	}

	AActor* beginActor = beginComponent->GetChildActor();
	AActor* endActor = endComponent->GetChildActor();
	if (!beginActor || !endActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s-Rope Component : BeginChildActor or EndChildActor is hasn't been created"), owner?*owner->GetName() : *FString("Error"));
		return;
	}

	URopeAttachmentComponent*	beginAttach = beginActor->FindComponentByClass<URopeAttachmentComponent>();
	URopeAttachmentComponent*	endAttach = endActor->FindComponentByClass<URopeAttachmentComponent>();
	if (!beginAttach || !endAttach)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s-Rope Component : BeginRopeAttachment or EndRopeAttachment is wrong"), owner ? *owner->GetName() : *FString("Error"));
		return;
	}

	beginAttachPrimitive = beginAttach->GetRopeAttachment();
	endAttachPrimitive = endAttach->GetRopeAttachment();
	if (!beginAttachPrimitive || !endAttachPrimitive)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s-Rope Component : BeginRopeAttachmentPrimitive or EndRopeAttachmentPrimitive is wrong"), owner ? *owner->GetName() : *FString("Error"));
		return;
	}

	FVector beginActorBoundsLoc, beginActorBoxExtent;
	beginActor->GetActorBounds(true, beginActorBoundsLoc, beginActorBoxExtent);
	FVector endActorBoundsLoc, endActorBoxExtent;
	endActor->GetActorBounds(true, endActorBoundsLoc, endActorBoxExtent);

	FVector	direction = (endComponent->GetComponentLocation() - beginComponent->GetComponentLocation()).GetSafeNormal();
	FVector	sphereBeginPoint = beginComponent->GetComponentLocation() + direction * beginActorBoxExtent;
	FVector	sphereEndPoint = endComponent->GetComponentLocation() + direction * endActorBoxExtent * -1.0f;

	float	finalLength = (sphereEndPoint - sphereBeginPoint).Size();

	int parts = (int)(finalLength / Thickness);
	if (parts == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s-Rope Component : Thickness is too high for the rope distance, should be inferior to %f"), owner ? *owner->GetName() : *FString("Error"), finalLength);
		return;
	}

	float rest = finalLength - (parts * Thickness);
	float padding = rest / (parts - 1);

	FVector	spawnPoint = sphereBeginPoint + direction * Thickness * 0.5f;
	
	for (int idx = 0; idx < parts; idx++)
	{
		USphereComponent*	sphere = NewObject<USphereComponent>(this);
		sphere->SetupAttachment(this);
		sphere->RegisterComponent();
		sphere->SetSphereRadius(Thickness * 0.5f, false);
		sphere->SetWorldLocation(spawnPoint);
		sphere->SetSimulatePhysics(true);
		sphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		sphere->SetCollisionProfileName("SmallInteractable");
		sphere->SetEnableGravity(true);
		sphere->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;

		spawnPoint += direction * (Thickness + padding);
		spheres.Add(sphere);
	}

	UPhysicsConstraintComponent* beginConstraint = NewObject<UPhysicsConstraintComponent>(this);
	beginConstraint->SetupAttachment(this);
	beginConstraint->RegisterComponent();
	beginConstraint->SetWorldLocation(sphereBeginPoint);
	beginConstraint->SetConstrainedComponents(beginAttachPrimitive, NAME_None, spheres[0], NAME_None);
	constraints.Add(beginConstraint);

	UPhysicsConstraintComponent* endConstraint = NewObject<UPhysicsConstraintComponent>(this);
	endConstraint->SetupAttachment(this);
	endConstraint->RegisterComponent();
	endConstraint->SetWorldLocation(sphereEndPoint);
	endConstraint->SetConstrainedComponents(endAttachPrimitive, NAME_None, spheres.Last(), NAME_None);
	constraints.Add(endConstraint);

	spline = NewObject<USplineComponent>(this);
	spline->SetupAttachment(this);
	spline->SetRelativeTransform(FTransform());
	spline->RegisterComponent();

	createConstraints();
	updateSplinePoints();
	createSplineMeshes();

	isInit = true;
}


// Called every frame
void URopeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!isInit)
		return;
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
	for (int pos = 0; pos < spheres.Num() - 1; pos++)
	{
		FVector	distanceNext = spheres[pos + 1]->GetComponentLocation() - spheres[pos]->GetComponentLocation();
		UPhysicsConstraintComponent* constraint = NewObject<UPhysicsConstraintComponent>(this);
		constraint->SetupAttachment(this);
		constraint->RegisterComponent();
		constraint->SetWorldLocation(spheres[pos]->GetComponentLocation() + distanceNext * 0.5f);
		constraint->SetConstrainedComponents(spheres[pos], NAME_None, spheres[pos + 1], NAME_None);
		constraint->SetAngularTwistLimit(EAngularConstraintMotion::ACM_Locked, 0.0f);
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

	splinePoints.Add(spheres[0]->GetComponentLocation() + Thickness * 0.5f * (beginAttachPrimitive->GetComponentLocation() - spheres[0]->GetComponentLocation()).GetSafeNormal());
	for (int pos = 0; pos < spheres.Num() - 1; pos++)
		splinePoints.Add(spheres[pos]->GetComponentLocation() + (spheres[pos + 1]->GetComponentLocation() - spheres[pos]->GetComponentLocation()) * 0.5f);
	splinePoints.Add(spheres.Last()->GetComponentLocation() + Thickness * 0.5f * (endAttachPrimitive->GetComponentLocation() - spheres.Last()->GetComponentLocation()).GetSafeNormal());
	spline->SetSplinePoints(splinePoints, ESplineCoordinateSpace::World, true);
}