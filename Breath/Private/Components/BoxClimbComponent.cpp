// Fill out your copyright notice in the Description page of Project Settings.

#include "BoxClimbComponent.h"

#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

void	UBoxClimbComponent::BeginPlay()
{ 
	Super::BeginPlay();

	FScriptDelegate	beginOverlapDelegate;
	beginOverlapDelegate.BindUFunction(this, "OnBeginOverlap");
	OnComponentBeginOverlap.AddUnique(beginOverlapDelegate);

	FScriptDelegate	endOverlapDelegate;
	endOverlapDelegate.BindUFunction(this, "OnEndOverlap");
	OnComponentEndOverlap.AddUnique(endOverlapDelegate);
}

void UBoxClimbComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	for (auto&& climbingComponent : climbingComponents)
	{
		if (!climbingComponent.IsValid())
			climbingComponents.Remove(climbingComponent);
	}
}

bool	UBoxClimbComponent::IsOverlappingOthers() const
{ 
	TArray<UPrimitiveComponent*>	overlappingComponents;
	GetOverlappingComponents(overlappingComponents);
	for (auto& comp : overlappingComponents)
	{
		bool	isSame = false;
		for (auto& climbComp : climbingComponents)
		{
			if (comp == climbComp)
			{
				isSame = true;
				break;
			}
		}
		if (isSame)
			return true;
	}
	return false;
}
	
bool	UBoxClimbComponent::CheckSpaceOver() const
{
	if (climbingComponents.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Authorized to climb something but there isn't any climbable surface"));
		return false;
	}
	AActor*	character = Cast<AActor>(GetOwner());
	if (!character)
		return false;
	UCapsuleComponent*	characterCapsule = character->FindComponentByClass<UCapsuleComponent>();
	if (!characterCapsule)
		return false;
	float	capsHeight = characterCapsule->GetScaledCapsuleHalfHeight() * 2.0f;
	FHitResult	hitRes;
	//DrawDebugLine(GetWorld(), climbingComponents[0]->GetComponentLocation(),
	//	climbingComponents[0]->GetComponentLocation() + climbingComponents[0]->GetUpVector() * capsHeight, FColor::Black, false, 10.0f, 0, 2.0f);
	FCollisionQueryParams	queryParams;
	queryParams.AddIgnoredComponent(climbingComponents[0].Get());
	bool result = GetWorld()->LineTraceSingleByChannel(hitRes, climbingComponents[0]->GetComponentLocation(),
		climbingComponents[0]->GetComponentLocation() + climbingComponents[0]->GetUpVector() * capsHeight,
		ECollisionChannel::ECC_PhysicsBody, queryParams);
	return !result;
}

FVector	UBoxClimbComponent::GetClimbedLocation() const
{
	if (climbingComponents.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Trying to get climbed location but there isn't any climbable surface"));
		return FVector();
	}
	/*
	for (auto&& overlappingComponent : OverlappingComponents)
	{
		if (overlappingComponent.OverlapInfo.GetComponent() == climbingComponents[0].Get())
		{
			UE_LOG(LogTemp, Warning, TEXT("GOTCHA"));
		}
	}
	*/
	FVector tempLoc;
	this->GetClosestPointOnCollision(climbingComponents[0]->GetComponentLocation(), tempLoc);
	DrawDebugSphere(GetWorld(), tempLoc, 1.0f, 50, FColor::Black, true, 10.0f);
	return tempLoc;
	//return climbingComponents[0]->GetComponentLocation();
}

void	UBoxClimbComponent::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	for (auto&& componentTag : OtherComp->ComponentTags)
	{
		if (componentTag == "ClimbSurface")
		{
			climbingComponents.Add(OtherComp);
			boxClimbOverlap.Broadcast();
			//Call
		}
	}
}

void	UBoxClimbComponent::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	for (int pos = 0; pos < climbingComponents.Num(); pos++)
	{
		if (climbingComponents[pos].IsValid())
		{
			if (OtherComp == climbingComponents[pos].Get())
			{
				climbingComponents[pos] = nullptr;
				climbingComponents.RemoveAt(pos);
				boxClimbEndOverlap.Broadcast();
				break;
			}
		}
	}
}