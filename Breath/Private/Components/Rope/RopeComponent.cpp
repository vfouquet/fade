// Fill out your copyright notice in the Description page of Project Settings.

#include "RopeComponent.h"

#include "InteractableComponent.h"
#include "RopeAttachmentComponent.h"
#include "Engine/StaticMesh.h"

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
		URopeNodeComponent*	node = NewObject<URopeNodeComponent>(this);
		node->SetupAttachment(this);
		node->RegisterComponent();
		nodes.Add(node);
		node->CreateSphere(Thickness * 0.5f, spawnPoint);

		spawnPoint += direction * (Thickness + padding);
	}

	if (CanBurn)
	{
		for (int idx = 0; idx < nodes.Num(); idx++)
		{
			if (idx == 0)
				nodes[idx]->CreateWoodProperty(beginAttachPrimitive, nodes[idx + 1]->GetSphere());
			else if (idx == nodes.Num() - 1)
				nodes[idx]->CreateWoodProperty(nodes[idx - 1]->GetSphere(), endAttachPrimitive);
			else
				nodes[idx]->CreateWoodProperty(nodes[idx - 1]->GetSphere(), nodes[idx + 1]->GetSphere());
		}
	}

	UPhysicsConstraintComponent* beginConstraint = NewObject<UPhysicsConstraintComponent>(this);
	beginConstraint->SetupAttachment(this);
	beginConstraint->RegisterComponent();
	beginConstraint->SetWorldLocation(sphereBeginPoint);
	beginConstraint->SetLinearXLimit(ELinearConstraintMotion::LCM_Locked, 5.0f);
	beginConstraint->SetConstrainedComponents(beginAttachPrimitive, NAME_None, nodes[0]->GetSphere(), NAME_None);
	nodes[0]->SetPreviousConstraint(beginConstraint);

	UPhysicsConstraintComponent* endConstraint = NewObject<UPhysicsConstraintComponent>(this);
	endConstraint->SetupAttachment(this);
	endConstraint->RegisterComponent();
	endConstraint->SetWorldLocation(sphereEndPoint);
	endConstraint->SetLinearXLimit(ELinearConstraintMotion::LCM_Locked, 5.0f);
	endConstraint->SetConstrainedComponents(endAttachPrimitive, NAME_None, nodes.Last()->GetSphere(), NAME_None);
	nodes.Last()->SetNextConstraint(endConstraint);

	spline = NewObject<USplineComponent>(this);
	spline->SetupAttachment(this);
	spline->SetRelativeTransform(FTransform());
	spline->RegisterComponent();

	createConstraints();
	createSplineMeshes();

	isInit = true;

	if (BeginComponentStickOverride.ComponentProperty != NAME_None)
		attachBeginPrimitive();
	if (EndComponentStickOverride.ComponentProperty != NAME_None)
		attachEndPrimitive();
}


// Called every frame
void URopeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!isInit)
		return;
	updateSplineMeshes();
}

void	URopeComponent::createSplineMeshes()
{
	FBox boxSize = mesh->GetBoundingBox();
	float yScale = Thickness / boxSize.GetSize().Y;
	float ZScale = Thickness / boxSize.GetSize().Z;

	TArray<FVector>	points;
	for (auto& node : nodes)
		points.Add(node->GetPreviousConstraintLocation());
	points.Add(nodes.Last()->GetNextConstraintLocation());
	spline->SetSplinePoints(points, ESplineCoordinateSpace::World, true);

	int idx = 0;
	for (auto& node : nodes)
	{
		USplineMeshComponent*	splineMesh = NewObject<USplineMeshComponent>(this);
		//splineMesh->SetupAttachment(this);
		splineMesh->RegisterComponent();
		FVector	startPoint;
		FVector	endPoint;
		FVector	startTangent;
		FVector	endTangent;
		spline->GetLocationAndTangentAtSplinePoint(idx, startPoint, startTangent, ESplineCoordinateSpace::World);
		spline->GetLocationAndTangentAtSplinePoint(idx + 1, endPoint, endTangent, ESplineCoordinateSpace::World);
		splineMesh->SetStartAndEnd(startPoint, startTangent, endPoint, endTangent, true);

		splineMesh->SetStartScale(FVector2D(yScale, ZScale));
		splineMesh->SetEndScale(FVector2D(yScale, ZScale));

		splineMesh->SetStaticMesh(mesh);
		node->SetSplineMesh(splineMesh);
		idx++;
	}
}

void	URopeComponent::createConstraints()
{
	for (int pos = 0; pos < nodes.Num() - 1; pos++)
	{
		FVector	distanceNext = nodes[pos + 1]->GetSphereLocation() - nodes[pos]->GetSphereLocation();
		UPhysicsConstraintComponent* constraint = NewObject<UPhysicsConstraintComponent>(this);
		constraint->SetupAttachment(this);
		constraint->RegisterComponent();
		constraint->SetWorldLocation(nodes[pos]->GetSphereLocation() + distanceNext * 0.5f);
		constraint->SetConstrainedComponents(nodes[pos]->GetSphere(), NAME_None, nodes[pos + 1]->GetSphere(), NAME_None);
		nodes[pos + 1]->SetPreviousConstraint(constraint);
		nodes[pos]->SetNextConstraint(constraint);
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
		points.Add(node->GetPreviousConstraintLocation());
		lastPoint = node->GetNextConstraintLocation();
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