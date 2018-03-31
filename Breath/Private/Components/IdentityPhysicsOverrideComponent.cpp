// Fill out your copyright notice in the Description page of Project Settings.

#include "IdentityPhysicsOverrideComponent.h"

#include "Components/PrimitiveComponent.h"

// Sets default values for this component's properties
UIdentityPhysicsOverrideComponent::UIdentityPhysicsOverrideComponent()
{
}


void UIdentityPhysicsOverrideComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	associatedComponent = Cast<UPrimitiveComponent>(PrimitiveComponent.GetComponent(GetOwner()));
}

void	UIdentityPhysicsOverrideComponent::EraseIdentity()
{
	if (!associatedComponent)
		return;
	bHasIdentity = false;

	if (OverrideSimulatePhysics)
	{
		basicSimulatedPhysicsValue = associatedComponent->IsSimulatingPhysics();
		associatedComponent->SetSimulatePhysics(SimulatedPhysicsValue);
	}
	if (OverrideProfileName)
	{
		basicProfilName = associatedComponent->GetCollisionProfileName();
		associatedComponent->SetCollisionProfileName(ProfileNameValue);
	}
	if (OverrideCollisionEnabled)
	{
		basicCollisionEnabledType = associatedComponent->GetCollisionEnabled();
		associatedComponent->SetCollisionEnabled(CollisionEnabledValue);
	}
}

void	UIdentityPhysicsOverrideComponent::GiveIdentity()
{
	if (!associatedComponent)
		return;
	bHasIdentity = true;

	if (OverrideSimulatePhysics)
		associatedComponent->SetSimulatePhysics(basicSimulatedPhysicsValue);
	if (OverrideProfileName)
		associatedComponent->SetCollisionProfileName(basicProfilName);
	if (OverrideCollisionEnabled)
		associatedComponent->SetCollisionEnabled(basicCollisionEnabledType);
}

UIdentityPhysicsOverrideComponent*	UIdentityPhysicsOverrideComponent::FindAssociatedPhysicsOverriderComponent(UPrimitiveComponent* reference)
{
	AActor* refCompOwner = reference->GetOwner();
	if (!refCompOwner)	return nullptr;

	TArray<UActorComponent*>	overrideComponents = refCompOwner->GetComponentsByClass(UIdentityPhysicsOverrideComponent::StaticClass());
	for (auto& actorComp : overrideComponents)
	{
		UIdentityPhysicsOverrideComponent*	overrideComp = Cast<UIdentityPhysicsOverrideComponent>(actorComp);
		if (!overrideComp) continue;
		if (overrideComp->GetAssociatedComponent() == reference)
			return overrideComp;
	}
	return nullptr;
}