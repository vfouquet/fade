// Fill out your copyright notice in the Description page of Project Settings.

#include "HoldComponent.h"

#include "Engine/World.h"
#include "InteractableComponent.h"
#include "DrawDebugHelpers.h"

#ifdef WITH_EDITOR
#include "UnrealEd.h"
#endif

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
	characterMoveComponent = GetOwner()->FindComponentByClass<UMoveComponent>();
	handleTargetLocation = Cast<UPrimitiveComponent>(HandleTargetLocationReference.GetComponent(GetOwner()));
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

	if (GetWorld()->SweepMultiByChannel(hitResults, handleTargetLocation->GetComponentLocation(),
		handleTargetLocation->GetComponentLocation(), quaternion,
		ECC_WorldDynamic, shape, queryParams, responseParam))
	{	
		for (auto& hitRes : hitResults)
		{
			if (!hitRes.Actor.IsValid())
				continue;
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
	else if (currentHoldingState == EHoldingState::HeavyGrabbing)
	{
#ifdef WITH_EDITOR
		ACharacter*	charac = Cast<ACharacter>(GetOwner());
		if (!charac)
			return;
		APlayerController*	cont = Cast<APlayerController>(charac->GetController());
		if (cont->IsInputKeyDown(EKeys::G))
			GUnrealEd->PlayWorld->bDebugPauseExecution = true;
#endif
	}
}

void	UHoldComponent::Action()
{
	if (currentHoldingState == EHoldingState::LightGrabbing)
	{
		if (closestInteractableObject.IsValid() && closestInteractableObject->CanAcceptStick)
			Stick();
		else
			Throw();
	}
	else if (currentHoldingState == EHoldingState::HeavyGrabbing)
		Throw();
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
		//holdingStateChangedDelegate.Broadcast(EHoldingState::None, EHoldingState::LightGrabbing);

		holdingObject = closestInteractableObject.Get();
		if (holdingObject->IsSticked())
			holdingObject->Unstick();
		AActor*	holdingActor = holdingObject->GetOwner();

		//MAYBE QLWAYS TRY THE ROOT AS PRIMITIVE
		holdingPrimitiveComponent =
			holdingActor->FindComponentByClass<UPrimitiveComponent>();
		handleComponent->GrabComponentAtLocationWithRotation
		(holdingPrimitiveComponent, "", holdingActor->GetActorLocation(), holdingActor->GetActorRotation());
	}
	else
	{
		currentHoldingState = EHoldingState::HeavyGrabbing;
		if (characterMoveComponent)
			characterMoveComponent->EnableMovingHeavyObjectMode();

		holdingObject = closestInteractableObject.Get();
		holdingPrimitiveComponent =
			holdingObject->GetOwner()->FindComponentByClass<UPrimitiveComponent>();
		createHandConstraint();

		//holdingStateChangedDelegate.Broadcast(EHoldingState::None, EHoldingState::HeavyGrabbing);
	}
}

void	UHoldComponent::StopGrab()
{
	if (currentHoldingState == EHoldingState::LightGrabbing)
	{
		releaseLightGrabbedObject();
		currentHoldingState = EHoldingState::None;
		//holdingStateChangedDelegate.Broadcast(EHoldingState::LightGrabbing, EHoldingState::None);
	}
	else if (currentHoldingState == EHoldingState::HeavyGrabbing)
	{
		//holdingStateChangedDelegate.Broadcast(EHoldingState::HeavyGrabbing, EHoldingState::None);
		if (characterMoveComponent)
			characterMoveComponent->DisableMovingHeavyObjectMode();

		releaseHeavyGrabbedObject();

		holdingObject = nullptr;
		holdingPrimitiveComponent = nullptr;
		currentHoldingState = EHoldingState::None;
	}
}

void	UHoldComponent::Throw()
{
	if (currentHoldingState == EHoldingState::LightGrabbing)
	{
		currentHoldingState = EHoldingState::Throwing;
		//holdingStateChangedDelegate.Broadcast(EHoldingState::LightGrabbing, EHoldingState::Throwing);

		//DO THIS AT THE END OF ANIMATION (NOTIFY)
		UPrimitiveComponent* tempPrimitive = holdingPrimitiveComponent;
		releaseLightGrabbedObject();
		tempPrimitive->AddImpulse(characterCapsule->GetForwardVector() * ThrowPower);
		//holdingStateChangedDelegate.Broadcast(EHoldingState::Throwing, EHoldingState::None);
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
		if (characterMoveComponent)
			characterMoveComponent->BlockCharacter();

		holdingObject->SetStickingActivated();
		closestInteractableObject->AddStickConstraint(holdingObject, holdingPrimitiveComponent, TEXT("None"));
		releaseLightGrabbedObject();
		//holdingStateChangedDelegate.Broadcast(EHoldingState::LightGrabbing, EHoldingState::Sticking);

		if (characterMoveComponent)
			characterMoveComponent->UnblockCharacter();
		currentHoldingState = EHoldingState::None;
		//holdingStateChangedDelegate.Broadcast(EHoldingState::Sticking, EHoldingState::None);
	}
}

void	UHoldComponent::releaseLightGrabbedObject()
{
	handleComponent->ReleaseComponent();
	holdingObject = nullptr;
	holdingPrimitiveComponent = nullptr;
}

void	UHoldComponent::createHandConstraint()
{
	leftHandConstraint = NewObject<UPhysicsConstraintComponent>(this, TEXT("Left Hand Constraint"));
	if (!leftHandConstraint)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not create left hand constraint"));
		return;
	}
	rightHandConstraint = NewObject<UPhysicsConstraintComponent>(this, TEXT("Right Hand Constraint"));
	if (!rightHandConstraint)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not create right hand constraint"));
		
		leftHandConstraint->DestroyComponent();
		return;
	}
	leftHandConstraint->SetupAttachment(GetOwner()->GetRootComponent());
	rightHandConstraint->SetupAttachment(GetOwner()->GetRootComponent());
	leftHandConstraint->RegisterComponent();
	rightHandConstraint->RegisterComponent();

	//TEMPORARY CHANGE WHEN CHARACTER HAVE BONE (USE BONE IN SET CONSTRAINED)
	
	FVector	leftHandLocation;
	FVector	rightHandLocation;
	getPushingPoints(leftHandLocation, rightHandLocation);

	leftHandConstraint->SetWorldLocation(leftHandLocation);
	rightHandConstraint->SetWorldLocation(rightHandLocation);
	//

	//leftHandConstraint->SetConstrainedComponents(characterCapsule, "None", holdingPrimitiveComponent, "None");
	leftHandConstraint->SetConstrainedComponents(handleTargetLocation, "None", holdingPrimitiveComponent, "None");
	//rightHandConstraint->SetConstrainedComponents(characterCapsule, "None", holdingPrimitiveComponent, "None");
	rightHandConstraint->SetConstrainedComponents(handleTargetLocation, "None", holdingPrimitiveComponent, "None");
	//leftHandConstraint->SetDisableCollision(true);
	//rightHandConstraint->SetDisableCollision(true);

	//characterCapsule->GetBodyInstance()->Weld(holdingPrimitiveComponent->GetBodyInstance(), holdingPrimitiveComponent->GetComponentTransform());
}

void	UHoldComponent::releaseHeavyGrabbedObject()
{
	if (leftHandConstraint)
	{
		leftHandConstraint->BreakConstraint();
		leftHandConstraint->DestroyComponent();
	}
	if (rightHandConstraint)
	{
		rightHandConstraint->BreakConstraint();
		rightHandConstraint->DestroyComponent();
	}
	//characterCapsule->GetBodyInstance()->UnWeld(holdingPrimitiveComponent->GetBodyInstance());
}

bool	UHoldComponent::getPushingPoints(FVector& firstPoint, FVector& secondPoint) const
{
	FVector	pointLoc;
	float dis = holdingPrimitiveComponent->GetClosestPointOnCollision(characterCapsule->GetOwner()->GetActorLocation(), pointLoc);
	if (dis == 0.0f)
		return false;
	FVector direction = pointLoc - characterCapsule->GetOwner()->GetActorLocation();
	FVector normal = FVector::CrossProduct(direction, characterCapsule->GetOwner()->GetActorUpVector()).GetUnsafeNormal();
	firstPoint = pointLoc + normal * characterCapsule->GetScaledCapsuleRadius();
	secondPoint = pointLoc - normal * characterCapsule->GetScaledCapsuleRadius();
	DrawDebugSphere(GetWorld(), firstPoint, 10.0f, 12, FColor::Emerald, true, 5.0f);
	DrawDebugSphere(GetWorld(), secondPoint, 10.0f, 12, FColor::Turquoise, true, 5.0f);
	return true;
}