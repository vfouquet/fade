// Fill out your copyright notice in the Description page of Project Settings.

#include "CharacterHealthComponent.h"

#include "MainCharacter.h"
#include "Components/CapsuleComponent.h"
#include "ChemicalComponent.h"
#include "Components/SkeletalMeshComponent.h"

#include "MainCharacterMovementComponent.h"

// Sets default values for this component's properties
UCharacterHealthComponent::UCharacterHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UCharacterHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	ACharacter*	character = Cast<ACharacter>(GetOwner());
	if (!character)
		return;
	UCapsuleComponent* capsule = character->GetCapsuleComponent();
	
	FScriptDelegate	beginOverlapDel;
	beginOverlapDel.BindUFunction(this, "OnCapsuleOverlap");
	capsule->OnComponentBeginOverlap.Add(beginOverlapDel);
	FScriptDelegate	endOverlapDel;
	endOverlapDel.BindUFunction(this, "OnCapsuleEndOverlap");
	capsule->OnComponentEndOverlap.Add(endOverlapDel);
	FScriptDelegate	hitOverlap;
	hitOverlap.BindUFunction(this, "OnCapsuleHit");
	capsule->OnComponentHit.Add(hitOverlap);
}


// Called every frame
void UCharacterHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UMainCharacterMovementComponent*	characterMovement = GetOwner()->FindComponentByClass<UMainCharacterMovementComponent>();
	bool ascending = false;
	if (characterMovement->IsFalling(ascending) && !ascending)
	{
		jumpZOffset -= characterMovement->GetLastMovementOffset().Z;
		if (jumpZOffset >= FatalJumpHeight)
			die();
	}
	else
		jumpZOffset = 0.0f;

	if (currentCondition != ECharacterCondition::None)
	{
		currentFireTime += DeltaTime;
		if (currentFireTime >= (currentCondition == ECharacterCondition::Scalding? ScaldingToBurning : BurningToDeath))
		{
			takeFireDamage();
			currentFireTime = 0.0f;
		}
	}
	if (currentDamageState == ECharacterDamageState::Wounded)
	{
		currentDamageTime += DeltaTime;
		if (currentDamageTime >= WoundedResetTime)
		{
			currentDamageTime = 0.0f;
			currentDamageState = ECharacterDamageState::None;
			onDamageStateChanged.Broadcast(ECharacterDamageState::Wounded, ECharacterDamageState::None);
		}
	}
}

void	UCharacterHealthComponent::OnCapsuleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	UChemicalComponent*	comp = UChemicalComponent::FindAssociatedChemicalComponent(OtherComp);
	if (!comp)
		return;
	if ((comp->GetType() == EChemicalType::Fire && comp->GetState() == EChemicalState::None) ||
		((comp->GetType() == EChemicalType::Rock || comp->GetType() == EChemicalType::Wood) && comp->GetState() == EChemicalState::Burning))
	{
		takeFireDamage();
		fireCount++;
	}
	else if (comp->GetType() == EChemicalType::Water)
		applyWaterEffect();
}

void	UCharacterHealthComponent::OnCapsuleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UChemicalComponent*	comp = UChemicalComponent::FindAssociatedChemicalComponent(OtherComp);
	if (!comp)
		return;
	if ((comp->GetType() == EChemicalType::Fire && comp->GetState() == EChemicalState::None) ||
			((comp->GetType() == EChemicalType::Rock || comp->GetType() == EChemicalType::Wood) && comp->GetState() == EChemicalState::Burning))
		decreaseFireCount();
}

void	UCharacterHealthComponent::OnCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	float impactForce = NormalImpulse.Size();
	if (impactForce >= ImpactForce2Threshold && impactForce < ImpactForce3Threshold)
	{
		if (currentDamageState == ECharacterDamageState::Wounded)
			die(Hit.ImpactNormal * ImpactMeshForce, Hit.Location, Hit.BoneName);
		else
		{
			currentDamageState = ECharacterDamageState::Wounded;
			onDamageStateChanged.Broadcast(ECharacterDamageState::None, ECharacterDamageState::Wounded);
		}
	}
	else if (impactForce >= ImpactForce3Threshold)
	{
		die(Hit.ImpactNormal * ImpactMeshForce, Hit.Location, Hit.BoneName);
	}
	UChemicalComponent*	comp = UChemicalComponent::FindAssociatedChemicalComponent(OtherComp);
	if (!comp)
		return;
	if ((comp->GetType() == EChemicalType::Fire && comp->GetState() == EChemicalState::None) ||
			((comp->GetType() == EChemicalType::Rock || comp->GetType() == EChemicalType::Wood) && comp->GetState() == EChemicalState::Burning))
		takeFireDamage();
}

void	UCharacterHealthComponent::takeFireDamage()
{
	if (currentCondition == ECharacterCondition::None)
	{
		currentCondition = ECharacterCondition::Scalding;
		onConditionChanged.Broadcast(ECharacterCondition::None, ECharacterCondition::Scalding);
	}
	else if (currentCondition == ECharacterCondition::Scalding)
	{
		currentCondition = ECharacterCondition::Burning;
		onConditionChanged.Broadcast(ECharacterCondition::Scalding, ECharacterCondition::Burning);
	}
	else if (currentCondition == ECharacterCondition::Burning)
	{
		die();
	}
}

void	UCharacterHealthComponent::decreaseFireCount()
{
	if (fireCount == 1)
	{
		fireCount = 0;
		if (currentCondition == ECharacterCondition::Scalding)
		{
			onConditionChanged.Broadcast(ECharacterCondition::Scalding, ECharacterCondition::None);
			currentCondition = ECharacterCondition::None;
			currentFireTime = 0.0f;
		}
	}
	else if (fireCount > 1)
		fireCount--;	
}

void	UCharacterHealthComponent::applyWaterEffect()
{
	if (currentCondition == ECharacterCondition::Scalding || currentCondition == ECharacterCondition::Burning)
	{
		onConditionChanged.Broadcast(currentCondition, ECharacterCondition::None);
		currentCondition = ECharacterCondition::None;
		currentFireTime = 0.0f;
	}
}

void	UCharacterHealthComponent::die(FVector impact, FVector impactLocation, FName boneName)
{
	AMainCharacter*	character = Cast<AMainCharacter>(GetOwner());
	if (!character)
		return;
	character->BlockCharacter();
	USkeletalMeshComponent*	skeletal = character->GetMesh();
	if (!skeletal)
		return;
	skeletal->SetSimulatePhysics(true);
	skeletal->WakeAllRigidBodies();
	if (boneName != NAME_None)
		skeletal->AddImpulse(impact, boneName);
	else
	{
		skeletal->AddImpulseAtLocation(impact, impactLocation);
	}
}