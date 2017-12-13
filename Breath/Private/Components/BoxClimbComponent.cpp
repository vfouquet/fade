// Fill out your copyright notice in the Description page of Project Settings.

#include "BoxClimbComponent.h"

void	UBoxClimbComponent::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	for (auto&& componentTag : OtherComp->ComponentTags)
	{
		if (componentTag == "ClimbSurface")
			climbingComponent = OtherComp;
	}
}

void	UBoxClimbComponent::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (climbingComponent.IsValid())
	{
		if (OverlappedComponent == climbingComponent.Get())
			climbingComponent = nullptr;
	}
}