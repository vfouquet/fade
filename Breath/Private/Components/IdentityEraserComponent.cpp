// Fill out your copyright notice in the Description page of Project Settings.

#include "IdentityEraserComponent.h"
#include "GameFramework/Character.h"

#include "ChemicalComponent.h"
#include "InteractableComponent.h"
#include "IdentityZoneManager.h"
#include "./Particles/ParticleSystemComponent.h"

void	UIdentityEraserComponent::BeginPlay()
{
	Super::BeginPlay();

	if (manager)
		manager->AddErasedZone(this);
	else
	{
		AActor* owner = GetOwner();
		UE_LOG(LogTemp, Warning, TEXT("%s - IdentityEraserComponent : Identity Zone Manager doesn't exist"), owner ? *owner->GetName() : *FString("Error"));
	}

	if (WaveParticleTemplate)
	{
		particleSystem = NewObject<UParticleSystemComponent>(this);
		if (!particleSystem)
		{
			AActor* owner = GetOwner();
			UE_LOG(LogTemp, Warning, TEXT("%s - IdentityEraserComponent : Couldn't create particle system"), owner? *owner->GetName() : *FString("Error"));
		}
		else
		{
			particleSystem->SetupAttachment(this);
			particleSystem->RegisterComponent();
			particleSystem->SetTemplate(WaveParticleTemplate);
		}
	}
	else
	{
		AActor* owner = GetOwner();
		UE_LOG(LogTemp, Warning, TEXT("%s - IdentityEraserComponent : Particle template for wave not set"), owner ? *owner->GetName() : *FString("Error"));
	}
}
	
void	UIdentityEraserComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (particleSystem)
	{
		FHitResult hitRes;
		FVector location = GetComponentLocation();
		FVector endTrace = location + FVector::UpVector * -500.0f;
		FCollisionQueryParams params;
		params.AddIgnoredActor(GetOwner());
		bool res = GetWorld()->LineTraceSingleByChannel(hitRes, location, endTrace, 
			ECollisionChannel::ECC_Visibility, params);
		particleSystem->SetWorldLocation(res? hitRes.Location + FVector(0.0f, 0.0f, 15.0f) : location);
	}
}

void	UIdentityEraserComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	Super::OnComponentDestroyed(bDestroyingHierarchy);
	if (manager)
		manager->RemoveZone(this);
}

void	UIdentityEraserComponent::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (!manager)
		return;

	if (ACharacter* character = Cast<ACharacter>(OtherActor))
		return;

	bool isPropertyExisting = false;
	int id = 0;
	AIdentityZoneManager::FErasedObjectProperties* outProperty = manager->containsErasedObjectProperties(OtherComp, isPropertyExisting, id);
	if (isPropertyExisting)
	{
		if (outProperty->erasedZoneNbr == 0)
			manager->updateObjectProperties(*outProperty, DecelerationTime);
		outProperty->erasedZoneNbr++;
	}
	else
	{
		if (Cast<ACharacter>(OtherActor))
			return;
		AIdentityZoneManager::FErasedObjectProperties&	properties = manager->createNewProperties(OtherComp, DecelerationTime);
		properties.erasedZoneNbr = 1;
	}
}

void	UIdentityEraserComponent::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!manager)
		return;

	if (ACharacter* character = Cast<ACharacter>(OtherActor))
		return;

	bool isPropertyExisting = false;
	int id = 0;
	AIdentityZoneManager::FErasedObjectProperties* outProperty = manager->containsErasedObjectProperties(OtherComp, isPropertyExisting, id);
	if (isPropertyExisting)
	{
		if (outProperty->memoryZoneNbr != 0)
			outProperty->erasedZoneNbr--;
		else if (outProperty->erasedZoneNbr == 1)
		{
			if (outProperty->physicsOverrider.IsValid())
				outProperty->physicsOverrider->GiveIdentity();
			else
			{
				if (outProperty->bWasSimulatingPhysics)
					OtherComp->SetSimulatePhysics(true);
			}
			if (outProperty->chemicalComponent.IsValid())
				outProperty->chemicalComponent->GiveIdentity(outProperty->previousChemicalState);
			if (outProperty->interactableComponent.IsValid())
				outProperty->interactableComponent->GiveIdentity();
			manager->RemoveAffectedObject(id);
		}
		else
			outProperty->erasedZoneNbr--;
	}
}