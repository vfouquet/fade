// Fill out your copyright notice in the Description page of Project Settings.

#include "InteractableComponent.h"
#include "AkGameplayStatics.h"
#include "GameFramework/Actor.h"
#include "HoldComponent.h"

#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "ChemicalCeramicComponent.h"
#include "GameFramework/Character.h"

// Sets default values for this component's properties
UInteractableComponent::UInteractableComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UInteractableComponent::BeginPlay()
{
	Super::BeginPlay();

	associatedComponent = Cast<UPrimitiveComponent>(Grab.GetComponent(GetOwner()));
	if (associatedComponent.IsValid())
	{
		FScriptDelegate	hitOverlap;
		hitOverlap.BindUFunction(this, "OnHit");
		associatedComponent->OnComponentHit.Add(hitOverlap);

		tempExtent = associatedComponent->Bounds.BoxExtent;
		
		if (StickAtLaunch.ComponentProperty != NAME_None)
		{
			UPrimitiveComponent* prim = Cast<UPrimitiveComponent>
				(StickAtLaunch.GetComponent(StickAtLaunch.OtherActor? StickAtLaunch.OtherActor : GetOwner()));
			if (!prim)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s-Interactable component : primitive reference incorrect"), *GetOwner()->GetName());
				return;
			}
			UInteractableComponent* interactable = FindAssociatedInteractableComponent(prim);
			if (!interactable)
			{
				UE_LOG(LogTemp, Warning, TEXT("%s-Interactable component : interactable reference couldn't be found from primitive component"), *GetOwner()->GetName());
				return;
			}
			AddStickConstraint(interactable, prim, NAME_None);
		}
	}	
	else
	{
		AActor* owner = GetOwner();
		UE_LOG(LogTemp, Warning, TEXT("%s - %s : Cannot find primitive component, reference is wrong"), owner? *owner->GetName() : *FString("Error"), *GetName());
	}
}


// Called every frame
void UInteractableComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool	UInteractableComponent::GetDebugLeft()
{
	if (!associatedComponent.IsValid())
		return false;

	FVector tempExtent2 = tempExtent * 1.0f;

	FCollisionShape shape = FCollisionShape::MakeBox(FVector(tempExtent2.X, 10.0f, tempExtent2.Z));
	FHitResult hitResult;

	FVector leftCenter = associatedComponent->GetComponentLocation() + associatedComponent->GetRightVector() * -tempExtent.Y;
	FCollisionQueryParams	params;
	params.AddIgnoredActor(GetOwner());
	DrawDebugBox(GetWorld(), leftCenter, FVector(tempExtent2.X, 10.0f, tempExtent2.Z), associatedComponent->GetComponentRotation().Quaternion(), FColor::Blue);
	bool res = GetWorld()->SweepSingleByChannel(hitResult, leftCenter, leftCenter, associatedComponent->GetComponentRotation().Quaternion(), ECollisionChannel::ECC_Visibility, shape, params);
	return res;
}

bool UInteractableComponent::TryToRotate(float Angle)
{
	return false;
}

bool	UInteractableComponent::CanRotateLeft(FVector characterForward)
{
	if (!associatedComponent.IsValid())
		return false;
	bool res = false;
	float dotRes = FVector::DotProduct(characterForward, associatedComponent->GetForwardVector());
	if (FMath::IsNearlyEqual(dotRes, 1.0f, 0.1f))
		res = !GetDebugLeft();
	else if (FMath::IsNearlyEqual(dotRes, -1.0f, 0.1f))
		res = !GetDebugRight();
	else
	{
		float dotResRight = FVector::DotProduct(characterForward, associatedComponent->GetRightVector());
		if (FMath::IsNearlyEqual(dotResRight, 1.0f, 0.1f))
			res = !GetDebugCenter();
		else if (FMath::IsNearlyEqual(dotResRight, -1.0f, 0.1f))
			res = !GetDebugBack();
	}
	return res;
}

bool	UInteractableComponent::CanRotateRight(FVector characterForward)
{
	if (!associatedComponent.IsValid())
		return false;
	bool res = false;
	float dotRes = FVector::DotProduct(characterForward, associatedComponent->GetForwardVector());
	if (FMath::IsNearlyEqual(dotRes, 1.0f, 0.1f))
		res = !GetDebugRight();
	else if (FMath::IsNearlyEqual(dotRes, -1.0f, 0.1f))
		res = !GetDebugLeft();
	else
	{
		float dotResRight = FVector::DotProduct(characterForward, associatedComponent->GetRightVector());
		if (FMath::IsNearlyEqual(dotResRight, 1.0f, 0.1f))
			res = !GetDebugBack();
		else if (FMath::IsNearlyEqual(dotResRight, -1.0f, 0.1f))
			res = !GetDebugCenter();
	}
	return res;
}

bool	UInteractableComponent::CanPushForward(FVector characterForward)
{
	if (!associatedComponent.IsValid())
		return false;
	bool res = false;
	float dotRes = FVector::DotProduct(characterForward, associatedComponent->GetForwardVector());
	if (FMath::IsNearlyEqual(dotRes, 1.0f, 0.1f))
		res = !GetDebugCenter();
	else if (FMath::IsNearlyEqual(dotRes, -1.0f, 0.1f))
		res = !GetDebugBack();
	else
	{
		float dotResRight = FVector::DotProduct(characterForward, associatedComponent->GetRightVector());
		if (FMath::IsNearlyEqual(dotResRight, 1.0f, 0.1f))
			res = !GetDebugRight();
		else if (FMath::IsNearlyEqual(dotResRight, -1.0f, 0.1f))
			res = !GetDebugLeft();
	}
	return res;
}

bool	UInteractableComponent::GetDebugRight()
{
	if (!associatedComponent.IsValid())
		return false;

	FVector	tempExtent2 = tempExtent * 1.0f;

	FCollisionShape shape = FCollisionShape::MakeBox(FVector(tempExtent2.X, 10.0f, tempExtent2.Z));
	FHitResult hitResult;

	FVector rightCenter = associatedComponent->GetComponentLocation() + associatedComponent->GetRightVector() * tempExtent.Y;
	FCollisionQueryParams	params;
	params.AddIgnoredActor(GetOwner());
	DrawDebugBox(GetWorld(), rightCenter, FVector(tempExtent2.X, 10.0f, tempExtent2.Z), associatedComponent->GetComponentRotation().Quaternion(), FColor::Blue);
	bool res = GetWorld()->SweepSingleByChannel(hitResult, rightCenter, rightCenter, associatedComponent->GetComponentRotation().Quaternion(), ECollisionChannel::ECC_Visibility, shape, params);
	return res;
}

bool	UInteractableComponent::GetDebugCenter()
{
	if (!associatedComponent.IsValid())
		return false;

	FVector	tempExtent2 = tempExtent * 1.0f;

	FCollisionShape shape = FCollisionShape::MakeBox(FVector(10.0f, tempExtent2.Y, tempExtent2.Z));
	FHitResult hitResult;

	FVector forwardCenter = associatedComponent->GetComponentLocation() + associatedComponent->GetForwardVector() * tempExtent.X;
	FCollisionQueryParams	params;
	params.AddIgnoredActor(GetOwner());
	DrawDebugBox(GetWorld(), forwardCenter, FVector(10.0f, tempExtent2.Y, tempExtent2.Z), associatedComponent->GetComponentRotation().Quaternion(), FColor::Blue);
	bool res = GetWorld()->SweepSingleByChannel(hitResult, forwardCenter, forwardCenter, associatedComponent->GetComponentRotation().Quaternion(), ECollisionChannel::ECC_Visibility, shape, params);
	return res;
}

bool	UInteractableComponent::GetDebugBack()
{
	if (!associatedComponent.IsValid())
		return false;

	FVector	tempExtent2 = tempExtent * 1.0f;

	FCollisionShape shape = FCollisionShape::MakeBox(FVector(10.0f, tempExtent2.Y, tempExtent2.Z));
	FHitResult hitResult;

	FVector forwardCenter = associatedComponent->GetComponentLocation() - associatedComponent->GetForwardVector() * tempExtent.X;
	FCollisionQueryParams	params;
	params.AddIgnoredActor(GetOwner());
	DrawDebugBox(GetWorld(), forwardCenter, FVector(10.0f, tempExtent2.Y, tempExtent2.Z), associatedComponent->GetComponentRotation().Quaternion(), FColor::Blue);
	bool res = GetWorld()->SweepSingleByChannel(hitResult, forwardCenter, forwardCenter, associatedComponent->GetComponentRotation().Quaternion(), ECollisionChannel::ECC_Visibility, shape, params);
	return res;
}

//#include "MainCharacter.h"

void	UInteractableComponent::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!associatedComponent.IsValid())
		return;
	if (!thrown && associatedComponent.IsValid())
	{
		UChemicalCeramicComponent* ceramicComp = Cast<UChemicalCeramicComponent>(UChemicalComponent::FindAssociatedChemicalComponent(associatedComponent.Get()));
		if (ceramicComp)
		{
			float	zVel = FMath::Abs(associatedComponent->GetComponentVelocity().Z);
			if (zVel > ceramicComp->HeightBreakThreshold)
				ceramicComp->Break();
			/*
			else if (OtherComp != nullptr && OtherComp->IsSimulatingPhysics())
			{
				if (OtherComp->GetComponentVelocity().Size() * OtherComp->GetMass() > ceramicComp->OtherActorThreshold)
					ceramicComp->Break();
			}
			else if (ACharacter* character = Cast<ACharacter>(OtherActor))
			{
				if (character->GetVelocity().Size() > 300.0f)
					ceramicComp->Break();
			}
			*/
		}
		return;
	}
	else
	{
		if (!Cast<ACharacter>(OtherActor))
		{
			UAkGameplayStatics::PostEvent(HitEvent, GetOwner());
			UChemicalCeramicComponent* ceramicComp = Cast<UChemicalCeramicComponent>(UChemicalComponent::FindAssociatedChemicalComponent(associatedComponent.Get()));
			if (ceramicComp)
				ceramicComp->Break();
			thrown = false;
		}
	}

	if (!CanBeSticked || !OtherActor || isSticked)
		return;
	UInteractableComponent*	otherInteractble = OtherActor->FindComponentByClass<UInteractableComponent>();
	if (!otherInteractble)
		return;
	if (otherInteractble->CanAcceptStick)
		otherInteractble->AddStickConstraint(this, HitComponent, NAME_None);
}
	
void	UInteractableComponent::ReceiveComponentDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{

}

void	UInteractableComponent::OnComponentDestroyed(bool bDestroyHierarchy)
{
	Super::OnComponentDestroyed(bDestroyHierarchy);

	if (holder)
	{
		holder->UniversalRelease();
		holder = nullptr;
	}
	for (auto& constraint : stickingConstraints)
	{
		constraint.physicConstraint->BreakConstraint();
		constraint.physicConstraint->DestroyComponent();
		constraint.carrier->RemoveHookingConstraint(this);
	}
	stickingConstraints.Empty();
}

void	UInteractableComponent::EraseIdentity()
{
	identityErased = true;
	thrown = false;
	if (holder)
	{
		holder->UniversalRelease();
		holder = nullptr;
	}
}

UPhysicsConstraintComponent*	UInteractableComponent::AddStickConstraint(UInteractableComponent* hook, UPrimitiveComponent* stickedObject, FName stickedBoneName)
{
	if (!CanAcceptStick)
	{
		AActor* owner = GetOwner();
		UE_LOG(LogTemp, Warning, TEXT("%s - Interactable : Trying to add a stick constraint but the component cannot accept a stick"), owner? *owner->GetName() : *FString("Error"));
		return nullptr;
	}
	if (!associatedComponent.IsValid() || !stickedObject)
	{
		AActor* owner = GetOwner();
		UE_LOG(LogTemp, Warning, TEXT("%s - Interactable : Trying to add a stick constraint but the associated component is null or the other one is null"), owner ? *owner->GetName() : *FString("Error"));
		return nullptr;
	}
	UPhysicsConstraintComponent* stickConstraint = NewObject<UPhysicsConstraintComponent>(this, TEXT("CustomPhysicConstraint"));
	if (!stickConstraint)
	{
		AActor* owner = GetOwner();
		UE_LOG(LogTemp, Error, TEXT("%s - Interactable : Could not create physical constraint"), owner ? *owner->GetName() :  *FString("Error"));
		return nullptr;
	}
	stickConstraint->SetupAttachment(this);
	stickConstraint->RegisterComponent();
	//stickConstraint->SetDisableCollision(true);
	FVector offset = associatedComponent->Bounds.BoxExtent * (stickedObject->GetComponentLocation() - associatedComponent->GetComponentLocation()).GetSafeNormal();
	stickConstraint->SetWorldLocation(associatedComponent->GetComponentLocation() + offset);
	stickConstraint->SetConstrainedComponents(associatedComponent.Get(), FName("None"), stickedObject, stickedBoneName);
	stickConstraint->SetDisableCollision(true);
	stickConstraint->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Locked, 0.0f);
	stickConstraint->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Locked, 0.0f);
	stickConstraint->SetAngularTwistLimit(EAngularConstraintMotion::ACM_Locked, 0.0f);

	FStickConstraint	constraint;
	constraint.physicConstraint = stickConstraint;
	constraint.carrier = this;
	constraint.hook = hook;
	stickingConstraints.Add(constraint);
	hook->stickingConstraints.Add(constraint);
	hook->isSticked = true;

	// FIX FOR ROPE <-> PUSH/PULL
	stickedObject->IgnoreActorWhenMoving(hook->GetOwner(), true);
	stickedObject->IgnoreActorWhenMoving(hook->GetOwner()->GetRootComponent()->GetAttachParent()->GetOwner(), true);
	hook->GetAssociatedComponent()->IgnoreActorWhenMoving(stickedObject->GetOwner(), true);
	hook->GetAssociatedComponent()->SetCollisionProfileName("OverlapAll");

	return stickConstraint;
}

void	UInteractableComponent::Unstick()
{
	if (!isSticked)
		return;
	for (int pos = 0; pos < stickingConstraints.Num(); pos++)
	{
		if (this == stickingConstraints[pos].hook)
		{
			if (stickingConstraints[pos].physicConstraint != nullptr)
			{ 
				stickingConstraints[pos].physicConstraint->BreakConstraint();
				stickingConstraints[pos].physicConstraint->DestroyComponent();
			}

			stickingConstraints[pos].carrier->RemoveHookingConstraint(this);

			// FIX FOR ROPE <-> PUSH/PULL
			stickingConstraints[pos].carrier->GetOwner()->GetRootPrimitiveComponent()->IgnoreActorWhenMoving(stickingConstraints[pos].hook->GetOwner(), false);
			stickingConstraints[pos].carrier->GetOwner()->GetRootPrimitiveComponent()->IgnoreActorWhenMoving(stickingConstraints[pos].hook->GetOwner()->GetRootComponent()->GetAttachParent()->GetOwner(), false);
			stickingConstraints[pos].hook->GetAssociatedComponent()->IgnoreActorWhenMoving(stickingConstraints[pos].carrier->GetOwner(), false);
			stickingConstraints[pos].hook->GetAssociatedComponent()->SetCollisionProfileName("SmallInteractable");


			stickingConstraints.RemoveAt(pos);

				//IgnoreActorWhenMoving(hook->GetOwner(), true);

			isSticked = false;
			return;
		}
	}
}

void	UInteractableComponent::RemoveHookingConstraint(UInteractableComponent* hookToRemove)
{
	for (int pos = 0; pos < stickingConstraints.Num(); pos++)
	{
		if (stickingConstraints[pos].hook == hookToRemove)
		{
			stickingConstraints.RemoveAt(pos);
			return;
		}
	}
}

UInteractableComponent* UInteractableComponent::FindAssociatedInteractableComponent(UPrimitiveComponent* referenceComponent)
{
	AActor* refCompOwner = referenceComponent->GetOwner();
	if (!refCompOwner)	return nullptr;

	TArray<UActorComponent*>	interactableComponents = refCompOwner->GetComponentsByClass(UInteractableComponent::StaticClass());
	for (auto& actorComp : interactableComponents)
	{
		UInteractableComponent*	interactableComp = Cast<UInteractableComponent>(actorComp);
		if (!interactableComp) continue;
		if (interactableComp->GetAssociatedComponent() == referenceComponent)
			return interactableComp;
	}
	return nullptr;
}