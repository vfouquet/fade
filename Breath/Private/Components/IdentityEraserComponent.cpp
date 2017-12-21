// Fill out your copyright notice in the Description page of Project Settings.

#include "IdentityEraserComponent.h"

#include "ChemicalComponent.h"

// Sets default values for this component's properties
UIdentityEraserComponent::UIdentityEraserComponent()
{
}

void	UIdentityEraserComponent::BeginPlay()
{
	Super::BeginPlay();

	FScriptDelegate	beginOverlapDelegate;
	beginOverlapDelegate.BindUFunction(this, "OnBeginOverlap");
	OnComponentBeginOverlap.AddUnique(beginOverlapDelegate);

	FScriptDelegate	endOverlapDelegate;
	endOverlapDelegate.BindUFunction(this, "OnEndOverlap");
	OnComponentEndOverlap.AddUnique(endOverlapDelegate);
}

void	UIdentityEraserComponent::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	UChemicalComponent*	otherChemical = OtherActor->FindComponentByClass<UChemicalComponent>();
	if (!otherChemical)
		return;
	otherChemical->EraseIdentity();
}

void	UIdentityEraserComponent::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UChemicalComponent*	otherChemical = OtherActor->FindComponentByClass<UChemicalComponent>();
	if (!otherChemical)
		return;
	otherChemical->GiveIdentity();
}