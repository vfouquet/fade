// Fill out your copyright notice in the Description page of Project Settings.

#include "HoldComponent.h"

#include "Engine/World.h"
#include "InteractableComponent.h"
#include "DrawDebugHelpers.h"

#include "UnrealEd.h"

// Sets default values for this component's properties
UHoldComponent::UHoldComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	handleComponent = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicHandleComponent"));
	// ...
}


// Called when the game starts
void UHoldComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
	characterCapsule = GetOwner()->FindComponentByClass<UCapsuleComponent>();
	handleTargetLocation = HandleTargetLocationReference.GetComponent(GetOwner());
}


// Called every frame
void UHoldComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	closestInteractableObject = nullptr;
	if (!handleTargetLocation && !characterCapsule)
		return;
	// ...
	float closestInteractable = FLT_MAX;

	TArray<FHitResult>			hitResults;
	FCollisionShape				shape;
	FCollisionQueryParams		queryParams;
	FCollisionResponseParams	responseParam;
	FQuat						quaternion = characterCapsule->GetComponentQuat();
	shape.SetCapsule(characterCapsule->GetScaledCapsuleRadius(), characterCapsule->GetScaledCapsuleHalfHeight());
	//DrawDebugCapsule(GetWorld(), handleTargetLocation->GetComponentLocation(), characterCapsule->GetScaledCapsuleHalfHeight(), characterCapsule->GetScaledCapsuleRadius(), quaternion, FColor::Blue);
	if (GetWorld()->SweepMultiByChannel(hitResults, handleTargetLocation->GetComponentLocation(),
		handleTargetLocation->GetComponentLocation(), quaternion,
		ECC_WorldDynamic, shape, queryParams, responseParam))
	{	
		for (auto& hitRes : hitResults)
		{
			UInteractableComponent* interComp = hitRes.Actor->FindComponentByClass<UInteractableComponent>();
			if (interComp == holdingObject)
				continue;
			if (interComp)
			{
				float distance = (hitRes.Actor->GetActorLocation() - characterCapsule->GetComponentLocation()).Size();
				if (distance < closestInteractable)
				{
					closestInteractable = distance;
					closestInteractableObject = interComp;
				}
			}
		}
	}

	if (currentHoldingState == EHoldingState::LightGrabbing)
		handleComponent->SetTargetLocation(handleTargetLocation->GetComponentLocation());
}

void	UHoldComponent::Grab()
{
	if (currentHoldingState != EHoldingState::None)
		return;
	if (!closestInteractableObject.IsValid())
		return;

	if (!closestInteractableObject->CanBeGrabbed)
		return;

	if (!closestInteractableObject->IsHeavy)
	{
		currentHoldingState = EHoldingState::LightGrabbing;
		holdingObject = closestInteractableObject.Get();
		AActor*	holdingActor = holdingObject->GetOwner();

		//MAYBE QLWAYS TRY THE ROOT AS PRIMITIVE
		holdingPrimitiveComponent =
			holdingActor->FindComponentByClass<UPrimitiveComponent>();
		handleComponent->GrabComponentAtLocationWithRotation
		(holdingPrimitiveComponent, "", holdingActor->GetActorLocation(), holdingActor->GetActorRotation());
	}
	else
	{
		
	}
}

void	UHoldComponent::StopGrab()
{
	if (currentHoldingState == EHoldingState::LightGrabbing)
	{
		releaseLightGrabbedObject();
		currentHoldingState = EHoldingState::None;
	}
}

void	UHoldComponent::Throw()
{
	if (currentHoldingState == EHoldingState::LightGrabbing)
	{
		currentHoldingState = EHoldingState::Throwing;

		//DO THIS AT THE END OF ANIMATION (NOTIFY)
		UPrimitiveComponent* tempPrimitive = holdingPrimitiveComponent;
		releaseLightGrabbedObject();
		tempPrimitive->AddImpulse(characterCapsule->GetForwardVector() * ThrowPower);
		currentHoldingState = EHoldingState::None;
	}
}

void	UHoldComponent::Stick()
{
	if (!holdingObject || !closestInteractableObject.IsValid())
		return;
	//ATTACH CONSTRAINT TO OTHER OBJECT
	if (currentHoldingState == EHoldingState::LightGrabbing)
	{
		if (!closestInteractableObject->CanAcceptStick || !holdingObject->CanBeSticked)
			return;

		currentHoldingState = EHoldingState::Sticking;
		UPrimitiveComponent* tempHoldingPrim = holdingPrimitiveComponent;
		releaseLightGrabbedObject();
		closestInteractableObject->AddStickConstraint(tempHoldingPrim, TEXT("None"));
		currentHoldingState = EHoldingState::None;
	}
}

void	UHoldComponent::releaseLightGrabbedObject()
{
	handleComponent->ReleaseComponent();
	holdingObject = nullptr;
	holdingPrimitiveComponent = nullptr;
}