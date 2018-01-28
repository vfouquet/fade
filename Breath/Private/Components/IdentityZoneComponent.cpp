// Fill out your copyright notice in the Description page of Project Settings.

#include "IdentityZoneComponent.h"

#include "Engine/World.h"

TArray<UIdentityZoneComponent::FErasedObjectProperties>	
	UIdentityZoneComponent::affectedObjects;

void	UIdentityZoneComponent::BeginPlay()
{
	Super::BeginPlay();

	FScriptDelegate	beginOverlapDelegate;
	beginOverlapDelegate.BindUFunction(this, "OnBeginOverlap");
	OnComponentBeginOverlap.AddUnique(beginOverlapDelegate);

	FScriptDelegate	endOverlapDelegate;
	endOverlapDelegate.BindUFunction(this, "OnEndOverlap");
	OnComponentEndOverlap.AddUnique(endOverlapDelegate);
}

void UIdentityZoneComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	for (auto& erasedObject : affectedObjects)
	{
		if (GetWorld()->RealTimeSeconds == erasedObject.lastTimeUpdated)
			continue;
		erasedObject.lastTimeUpdated = GetWorld()->RealTimeSeconds;
		if (!erasedObject.bDecelerating)
			continue;
		erasedObject.currentDecelerationTime += DeltaTime;
		if (erasedObject.currentDecelerationTime >= erasedObject.maxDeceleratingTime)
		{
			erasedObject.bDecelerating = false;
			erasedObject.primitiveComponent->ComponentVelocity = FVector::ZeroVector;
			erasedObject.primitiveComponent->SetSimulatePhysics(false);
		}
		else
			erasedObject.primitiveComponent->SetPhysicsLinearVelocity(FMath::Lerp(erasedObject.initialVelocity, FVector::ZeroVector,
				erasedObject.currentDecelerationTime / erasedObject.maxDeceleratingTime));
	}
}

bool	UIdentityZoneComponent::containsErasedObjectProperties(UPrimitiveComponent* reference, FErasedObjectProperties& outProperty, int& outId)
{
	for (int pos = 0; pos < affectedObjects.Num(); pos++)
	{
		if (affectedObjects[pos].primitiveComponent == reference)
		{
			outProperty = affectedObjects[pos];
			outId = pos;
			return true;
		}
	}
	return false;
}