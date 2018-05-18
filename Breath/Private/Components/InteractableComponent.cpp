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

void	UInteractableComponent::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!associatedComponent.IsValid() || !thrown)
		return;
		
	if (!Cast<ACharacter>(OtherActor))
	{
		UAkGameplayStatics::PostEvent(HitEvent, GetOwner());
		UChemicalCeramicComponent* ceramicComp = Cast<UChemicalCeramicComponent>(UChemicalComponent::FindAssociatedChemicalComponent(associatedComponent.Get()));
		if (ceramicComp)
			ceramicComp->Break();
		thrown = false;
		associatedComponent->SetCollisionResponseToChannel(ECC_Pawn, previousReponse);
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
		if (constraint.physicConstraint != nullptr)
		{
			constraint.physicConstraint->BreakConstraint();
			constraint.physicConstraint->DestroyComponent();
		}

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
// 			stickingConstraints[pos].carrier->GetOwner()->GetRootPrimitiveComponent()->IgnoreActorWhenMoving(stickingConstraints[pos].hook->GetOwner(), false);
// 			stickingConstraints[pos].carrier->GetOwner()->GetRootPrimitiveComponent()->IgnoreActorWhenMoving(stickingConstraints[pos].hook->GetOwner()->GetRootComponent()->GetAttachParent()->GetOwner(), false);
// 			stickingConstraints[pos].hook->GetAssociatedComponent()->IgnoreActorWhenMoving(stickingConstraints[pos].carrier->GetOwner(), false);
// 			stickingConstraints[pos].hook->GetAssociatedComponent()->SetCollisionProfileName("SmallInteractable");


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

void	UInteractableComponent::SetThrown()
{
	thrown = true;
	if (associatedComponent.IsValid())
	{
		previousReponse = associatedComponent->GetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn);
		associatedComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	}
}
	
TArray<UInteractableComponent*>	UInteractableComponent::GetCarrier() const
{
	TArray<UInteractableComponent*>	ret;
	for (int pos = 0; pos < stickingConstraints.Num(); pos++)
		ret.Add(stickingConstraints[pos].carrier);
	return ret;
}