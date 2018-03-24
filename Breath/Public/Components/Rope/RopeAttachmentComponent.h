// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/PrimitiveComponent.h"

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RopeAttachmentComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BREATH_API URopeAttachmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	URopeAttachmentComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPrimitiveComponent*	GetRopeAttachment() { return Cast<UPrimitiveComponent>(PrimitiveToAttachAtRope.GetComponent(GetOwner())); }

public:
	UPROPERTY(EditAnywhere, Category = "Rope")
	FComponentReference	PrimitiveToAttachAtRope;
};
