// Fill out your copyright notice in the Description page of Project Settings.

#include "RopeNodeComponent.h"

#include "Components/SplineMeshComponent.h"

// Sets default values for this component's properties
URopeNodeComponent::URopeNodeComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void URopeNodeComponent::BeginPlay()
{
	Super::BeginPlay();

	
}


// Called every frame
void URopeNodeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}
	
void URopeNodeComponent::OnComponentDestroyed(bool destroyedHierarchy)
{
	if (previousConstraint.IsValid())
		previousConstraint->DestroyComponent();
	if (nextConstraint.IsValid())
		nextConstraint->DestroyComponent();
	if (woodComponent)
		woodComponent->DestroyComponent();
	if (sphere)
		sphere->DestroyComponent();
	if (splineMesh)
		splineMesh->DestroyComponent();
}

void	URopeNodeComponent::CreateSphere(float size, FVector location)
{
	sphere = NewObject<USphereComponent>(this);
	sphere->SetupAttachment(this);
	sphere->RegisterComponent();
	sphere->SetSphereRadius(size, false);
	sphere->SetWorldLocation(location);
	sphere->SetSimulatePhysics(true);
	sphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	sphere->SetCollisionProfileName("Rope");
	sphere->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	sphere->SetMassOverrideInKg(NAME_None, size * 0.2f);
}
	
UChemicalWoodComponent*	URopeNodeComponent::CreateWoodProperty(UPrimitiveComponent* prevPrim, UPrimitiveComponent* nextPrim)
{
	woodComponent = NewObject<UChemicalWoodComponent>(GetOwner());
	woodComponent->RegisterComponent();
	if (prevPrim)
	{
		FComponentReference reference;
		reference.OverrideComponent = prevPrim;
		woodComponent->StaticPropagationComponentReferences.Add(reference);
	}
	if (nextPrim)
	{
		FComponentReference reference;
		reference.OverrideComponent = nextPrim;
		woodComponent->StaticPropagationComponentReferences.Add(reference);
	}
	woodComponent->OverrideAssociatedComponent(sphere);
	FScriptDelegate	woodDel;
	woodDel.BindUFunction(this, "onSphereChemicalStateChanged");
	woodComponent->stateChangedDelegate.Add(woodDel);
	return woodComponent;
}
	
void	URopeNodeComponent::UpdateSplineMesh(FVector startPoint, FVector startTangent, FVector endPoint, FVector endTangent)
{
	if (splineMesh)
		splineMesh->SetStartAndEnd(startPoint, startTangent, endPoint, endTangent, true);
}

void	URopeNodeComponent::SetPreviousPrimitive(UPrimitiveComponent* primitive, bool isExtremity)
{
	previousPrimitive = primitive;
	bIsPreviousExtremity = isExtremity;
}

void	URopeNodeComponent::SetNextPrimitive(UPrimitiveComponent* primitive, bool isExtremity)
{
	nextPrimitive = primitive;
	bIsNextExtremity = isExtremity;
}

FVector	URopeNodeComponent::GetPreviousSplinePointLocation() const
{
	if (previousConstraint.IsValid())
	{
		if (bIsPreviousExtremity)
		{
			FVector	direction = (sphere->GetComponentLocation() - previousPrimitive->GetComponentLocation()).GetSafeNormal();
			return sphere->GetComponentLocation() - direction * sphere->GetScaledSphereRadius();
		}
		return previousPrimitive->GetComponentLocation();
	}
	if (!sphere)
		return FVector::ZeroVector;
	FVector dir = nextConstraint.IsValid() ? (nextConstraint->GetComponentLocation() - sphere->GetComponentLocation()).GetSafeNormal()
		: sphere->GetComponentRotation().Vector();
	dir *= -1.0f;
	return sphere->GetComponentLocation() + dir * sphere->GetScaledSphereRadius();
}

FVector	URopeNodeComponent::GetNextSplinePointLocation() const
{
	if (nextConstraint.IsValid())
	{
		if (bIsNextExtremity)
		{
			FVector	direction = (sphere->GetComponentLocation() - nextPrimitive->GetComponentLocation()).GetSafeNormal();
			return sphere->GetComponentLocation() - direction * sphere->GetScaledSphereRadius();
		}
		return nextPrimitive->GetComponentLocation();
	}
	if (!sphere)
		return FVector::ZeroVector;
	FVector dir = previousConstraint.IsValid() ? (previousConstraint->GetComponentLocation() - sphere->GetComponentLocation()).GetSafeNormal() * -1.0f
		: sphere->GetComponentRotation().Vector();
	return sphere->GetComponentLocation() + dir * sphere->GetScaledSphereRadius();
}
	
void	URopeNodeComponent::onSphereChemicalStateChanged(EChemicalTransformation transformation, EChemicalState previous, EChemicalState next) 
{
	if (transformation == EChemicalTransformation::Burning && next == EChemicalState::Scorched)
		DestroyComponent();
}