// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerCameraComponent.h"




UPlayerCameraComponent::UPlayerCameraComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetComponentTickEnabled(true);
}

void UPlayerCameraComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (LookAtTarget != nullptr)
	{
		FVector Direction = LookAtTarget->GetComponentLocation() - this->GetComponentLocation();
		Direction.Normalize();

		this->SetWorldRotation(FMath::Lerp(this->GetComponentRotation(), Direction.Rotation(), 1.f));
	}
}

void UPlayerCameraComponent::OnAttachmentChanged()
{
	LookAtTarget = this->GetAttachParent();
}
