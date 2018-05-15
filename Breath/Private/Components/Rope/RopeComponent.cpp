// Fill out your copyright notice in the Description page of Project Settings.

#include "RopeComponent.h"

#include "InteractableComponent.h"
#include "RopeAttachmentComponent.h"
#include "Engine/StaticMesh.h"
#include "ChemicalWoodComponent.h"

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

	float parts = 0.0f;
	if (UsePrecisionPercentage)
		parts = (finalLength / Thickness) * (1.0f + PrecisionPercentage * 0.01f);
	else
		parts = finalLength / Thickness;
	int intParts = FMath::FloorToInt(parts);
	if (parts == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s-Rope Component : Thickness is too high for the rope distance, should be inferior to %f"), owner ? *owner->GetName() : *FString("Error"), finalLength);
		return;
	}

	float padding = 0.0f;
	if (UsePrecisionPercentage)
	{
		float rest = finalLength - ((parts * Thickness) * (1.0f - PrecisionPercentage * 0.01f));
		padding = rest / (parts - 1);
	}
	else
	{
		float rest = finalLength - (parts * Thickness);
		padding = rest / (parts - 1);
	}

	FVector	spawnPoint = sphereBeginPoint + direction * Thickness * 0.5f;
	
	for (int idx = 0; idx < intParts; idx++)
	{
		URopeNodeComponent*	node = NewObject<URopeNodeComponent>(this);
// 		if (idx > 0)
// 		{
// 			node->setup
// 		}
// 		else
		{
			node->SetupAttachment(this);
		}
		node->RegisterComponent();
		nodes.Add(node);
		node->CreateSphere(Thickness * 0.5f, spawnPoint);

		spawnPoint += UsePrecisionPercentage ? direction * ((Thickness + padding) - Thickness * PrecisionPercentage * 0.01f) :
			direction * (Thickness + padding);
	}

	UPhysicsConstraintComponent* beginConstraint = NewObject<UPhysicsConstraintComponent>(this);
	beginConstraint->SetupAttachment(this);
	beginConstraint->RegisterComponent();
	beginConstraint->SetWorldLocation(sphereBeginPoint);
	beginConstraint->SetConstrainedComponents(beginAttachPrimitive, NAME_None, nodes[0]->GetSphere(), NAME_None);
	beginConstraint->SetDisableCollision(true);
	beginConstraint->SetAngularVelocityDriveSLERP(true);
	beginConstraint->SetAngularDriveParams(0.0f, AngularMotorStrength, 0.0f);
	
	nodes[0]->SetPreviousConstraint(beginConstraint);
	nodes[0]->SetPreviousPrimitive(beginAttachPrimitive, true);

	UPhysicsConstraintComponent* endConstraint = NewObject<UPhysicsConstraintComponent>(this);
	endConstraint->SetupAttachment(this);
	endConstraint->RegisterComponent();
	endConstraint->SetWorldLocation(sphereEndPoint);
	endConstraint->SetConstrainedComponents(endAttachPrimitive, NAME_None, nodes.Last()->GetSphere(), NAME_None);
	endConstraint->SetDisableCollision(true);
	endConstraint->SetAngularVelocityDriveSLERP(true);
	endConstraint->SetAngularDriveParams(0.0f, AngularMotorStrength, 0.0f);

	nodes.Last()->SetNextConstraint(endConstraint);
	nodes.Last()->SetNextPrimitive(endAttachPrimitive, true);

	spline = NewObject<USplineComponent>(this);
	spline->SetupAttachment(this);
	spline->SetRelativeTransform(FTransform());
	spline->RegisterComponent();

	createConstraints();
	createSplineMeshes();
}


// Called every frame
void URopeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!isInit)
	{
		postInit();
		return;
	}
	updateSplineMeshes();
}

void	URopeComponent::createSplineMeshes()
{
	if (!RopeMesh)
	{
		AActor* owner = GetOwner();
		UE_LOG(LogTemp, Warning, TEXT("%s - Rope Component : Rope mesh not set"), *owner->GetName());
		return;
	}
	FBox boxSize = RopeMesh->GetBoundingBox();
	float yScale = Thickness / boxSize.GetSize().Y;
	float ZScale = Thickness / boxSize.GetSize().Z;

	TArray<FVector>	points;
	for (auto& node : nodes)
		points.Add(node->GetPreviousSplinePointLocation());
	points.Add(nodes.Last()->GetNextSplinePointLocation());
	spline->SetSplinePoints(points, ESplineCoordinateSpace::World, true);

	int idx = 0;
	for (auto& node : nodes)
	{
		USplineMeshComponent*	splineMesh = NewObject<USplineMeshComponent>(this);
		splineMesh->RegisterComponent();
		splineMesh->SetMobility(EComponentMobility::Movable);
		FVector	startPoint;
		FVector	endPoint;
		FVector	startTangent;
		FVector	endTangent;
		spline->GetLocationAndTangentAtSplinePoint(idx, startPoint, startTangent, ESplineCoordinateSpace::World);
		spline->GetLocationAndTangentAtSplinePoint(idx + 1, endPoint, endTangent, ESplineCoordinateSpace::World);
		splineMesh->SetStartAndEnd(startPoint, startTangent, endPoint, endTangent, true);

		splineMesh->SetStartScale(FVector2D(yScale, ZScale));
		splineMesh->SetEndScale(FVector2D(yScale, ZScale));

		if (node == nodes[0] && UseExtremityMesh)
		{
			splineMesh->SetStaticMesh(ExtrimityMesh);
			splineMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, -1.0f));
		}
		else if (node == nodes.Last() && UseExtremityMesh)
			splineMesh->SetStaticMesh(ExtrimityMesh);
		else
			splineMesh->SetStaticMesh(RopeMesh);
		splineMesh->SetMaterial(0, Material);
		//TO DO BETTER OPTI
		splineMesh->CastShadow = 0;

		node->SetSplineMesh(splineMesh);
		idx++;
	}
}

void	URopeComponent::postInit()
{
	if (CanBurn)
	{
		for (int idx = 0; idx < nodes.Num(); idx++)
		{
			UChemicalWoodComponent* tempWood = nullptr;
			if (idx == 0)
				tempWood = nodes[idx]->CreateWoodProperty(beginAttachPrimitive, nodes[idx + 1]->GetSphere(), RopeNodeStateChangedDelegate);
			else if (idx == nodes.Num() - 1)
				tempWood = nodes[idx]->CreateWoodProperty(nodes[idx - 1]->GetSphere(), endAttachPrimitive, RopeNodeStateChangedDelegate);
			else
				tempWood = nodes[idx]->CreateWoodProperty(nodes[idx - 1]->GetSphere(), nodes[idx + 1]->GetSphere(), RopeNodeStateChangedDelegate);
			tempWood->normalToLit = normalToLit;
			tempWood->litToBurning = litToBurning;
			tempWood->burningToScorched = burningToScorched;
		}
	}

	if (BeginComponentStickOverride.ComponentProperty != NAME_None)
		attachBeginPrimitive();
	if (EndComponentStickOverride.ComponentProperty != NAME_None)
		attachEndPrimitive();

	isInit = true;
}

void	URopeComponent::createConstraints()
{
	for (int pos = 0; pos < nodes.Num() - 1; pos++)
	{
		nodes[pos + 1]->GetSphere()->IgnoreActorWhenMoving(nodes[pos]->GetSphere()->GetOwner(), true);
		nodes[pos]->GetSphere()->IgnoreActorWhenMoving(nodes[pos + 1]->GetSphere()->GetOwner(), true);
		FVector	distanceNext = nodes[pos + 1]->GetSphereLocation() - nodes[pos]->GetSphereLocation();
		UPhysicsConstraintComponent* constraint = NewObject<UPhysicsConstraintComponent>(this);
		constraint->SetupAttachment(this);
		constraint->RegisterComponent();
		constraint->SetWorldLocation(nodes[pos]->GetSphereLocation() + distanceNext * 0.5f);
		constraint->SetConstrainedComponents(nodes[pos]->GetSphere(), NAME_None, nodes[pos + 1]->GetSphere(), NAME_None);
		if (UsePrecisionPercentage)
			constraint->SetDisableCollision(true);
		//constraint->SetAngularVelocityDriveSLERP(true);
		//constraint->SetAngularDriveParams(0.0f, AngularMotorStrength, 0.0f);
		//constraint->SetLinearVelocityDrive(true, true, true);
		//constraint->SetLinearDriveParams(1.f, 100.f, 1.f);

		nodes[pos + 1]->SetPreviousConstraint(constraint);
		nodes[pos + 1]->SetPreviousPrimitive(nodes[pos]->GetSphere());
		nodes[pos]->SetNextConstraint(constraint);
		nodes[pos]->SetNextPrimitive(nodes[pos + 1]->GetSphere());
	}	
}

void	URopeComponent::updateSplineMeshes()
{
	TArray<FVector>	points;

	int beginIdx = 0;
	FVector	lastPoint = FVector::ZeroVector;
	for (auto& node : nodes)
	{
		if (!node.IsValid())
		{
			points.Add(lastPoint);
			spline->SetSplinePoints(points, ESplineCoordinateSpace::World, true);
			
			for (int pos = 0; pos < points.Num() - 1; pos++)
			{
				FVector	startPoint;
				FVector	endPoint;
				FVector	startTangent;
				FVector	endTangent;
				spline->GetLocationAndTangentAtSplinePoint(pos, startPoint, startTangent, ESplineCoordinateSpace::World);
				spline->GetLocationAndTangentAtSplinePoint(pos + 1, endPoint, endTangent, ESplineCoordinateSpace::World);
				nodes[beginIdx]->UpdateSplineMesh(startPoint, startTangent, endPoint, endTangent);
				beginIdx++;
			}
			points.Empty();
			beginIdx++;
			continue;
		}
		points.Add(node->GetPreviousSplinePointLocation());
		lastPoint = node->GetNextSplinePointLocation();
	}
	points.Add(lastPoint);
	spline->SetSplinePoints(points, ESplineCoordinateSpace::World, true);
	
	for (int pos = 0; pos < points.Num() - 1; pos++)
	{
		FVector	startPoint;
		FVector	endPoint;
		FVector	startTangent;
		FVector	endTangent;
		spline->GetLocationAndTangentAtSplinePoint(pos, startPoint, startTangent, ESplineCoordinateSpace::World);
		spline->GetLocationAndTangentAtSplinePoint(pos + 1, endPoint, endTangent, ESplineCoordinateSpace::World);
		nodes[beginIdx]->UpdateSplineMesh(startPoint, startTangent, endPoint, endTangent);
		beginIdx++;
	}
}

void	URopeComponent::attachBeginPrimitive()
{
	UPrimitiveComponent*	otherPrimitive = Cast<UPrimitiveComponent>
		(BeginComponentStickOverride.GetComponent(BeginComponentStickOverride.OtherActor ? BeginComponentStickOverride.OtherActor : GetOwner()));
	if (!otherPrimitive)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s-rope component : Couldn't find other primitive for begin stick override"), *GetOwner()->GetName());
		return;
	}
	UInteractableComponent* interactable = UInteractableComponent::FindAssociatedInteractableComponent(beginAttachPrimitive);
	if (!interactable)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s-rope component : Couldn't find interactable component of BeginPrimitiveAttachment"), *GetOwner()->GetName());
		return;
	}
	UInteractableComponent* otherInteractable = UInteractableComponent::FindAssociatedInteractableComponent(otherPrimitive);
	if (!otherInteractable)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s-rope component : Couldn't find other interactable component of BeginPrimitiveAttachment"), *GetOwner()->GetName());
		return;
	}
	otherInteractable->AddStickConstraint(interactable, beginAttachPrimitive, NAME_None);

}

void	URopeComponent::attachEndPrimitive()
{
	UPrimitiveComponent*	otherPrimitive = Cast<UPrimitiveComponent>
		(EndComponentStickOverride.GetComponent(EndComponentStickOverride.OtherActor ? EndComponentStickOverride.OtherActor : GetOwner()));
	if (!otherPrimitive)
	{	
		UE_LOG(LogTemp, Warning, TEXT("%s-rope component : Couldn't find other primitive for end stick override"), *GetOwner()->GetName());
		return;
	}
	UInteractableComponent* interactable = UInteractableComponent::FindAssociatedInteractableComponent(endAttachPrimitive);
	if (!interactable)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s-rope component : Couldn't find interactable component of EndPrimitiveAttachment"), *GetOwner()->GetName());
		return;
	}
	UInteractableComponent* otherInteractable = UInteractableComponent::FindAssociatedInteractableComponent(otherPrimitive);
	if (!otherInteractable)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s-rope component : Couldn't find other interactable component of EndPrimitiveAttachment"), *GetOwner()->GetName());
		return;
	}
	otherInteractable->AddStickConstraint(interactable, endAttachPrimitive, NAME_None);
}

void	URopeComponent::destroyRope()
{
	if (!isInit)
		return;
	isInit = false;

	for (auto& node : nodes)
	{
		if (node.IsValid())
			node->DestroyComponent();
	}
	nodes.Empty();
	spline->DestroyComponent();
}