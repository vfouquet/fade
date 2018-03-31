// Fill out your copyright notice in the Description page of Project Settings.

#include "IdentityZoneComponent.h"

#include "Engine/World.h"
#include "IdentityEraserComponent.h"
#include "IdentityZoneManager.h"
#include "EngineUtils.h"

void	UIdentityZoneComponent::BeginPlay()
{
	Super::BeginPlay();

	for (TActorIterator<AIdentityZoneManager> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		manager = (*ActorItr);
		break;
	}

	FScriptDelegate	beginOverlapDelegate;
	beginOverlapDelegate.BindUFunction(this, "OnBeginOverlap");
	OnComponentBeginOverlap.AddUnique(beginOverlapDelegate);

	FScriptDelegate	endOverlapDelegate;
	endOverlapDelegate.BindUFunction(this, "OnEndOverlap");
	OnComponentEndOverlap.AddUnique(endOverlapDelegate);
}
	
void	UIdentityZoneComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bAlreadyTick)
	{
		bAlreadyTick = true;
		TArray<UPrimitiveComponent*>	primitives;
		GetOverlappingComponents(primitives);

		for (auto& prim : primitives)
		{
			AActor* otherActor = prim->GetOwner();
			if (otherActor != GetOwner())
				OnBeginOverlap(nullptr, otherActor, prim, 0, false, FHitResult()); //DANGEROUS SHIT
		}
	}
}