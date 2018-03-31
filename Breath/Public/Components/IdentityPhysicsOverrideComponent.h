// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IdentityPhysicsOverrideComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BREATH_API UIdentityPhysicsOverrideComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UIdentityPhysicsOverrideComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	virtual void	EraseIdentity();
	virtual void	GiveIdentity();

	static	UIdentityPhysicsOverrideComponent*	FindAssociatedPhysicsOverriderComponent(UPrimitiveComponent* reference);

	UPrimitiveComponent* GetAssociatedComponent() const { return associatedComponent; }

public:
	UPROPERTY(EditAnywhere, Category = "Identity")
	FComponentReference	PrimitiveComponent;

	UPROPERTY(EditAnywhere, Category = "Override Options")
	bool	OverrideSimulatePhysics = false;
	UPROPERTY(EditAnywhere, Category = "Override Options")
	bool	SimulatedPhysicsValue = false;
	UPROPERTY(EditAnywhere, Category = "Override Options")
	bool	OverrideProfileName = false;
	UPROPERTY(EditAnywhere, Category = "Override Options")
	FName	ProfileNameValue;
	UPROPERTY(EditAnywhere, Category = "Override Options")
	bool	OverrideCollisionEnabled = false;
	UPROPERTY(EditAnywhere, Category = "Override Options")
	TEnumAsByte<ECollisionEnabled::Type>	CollisionEnabledValue;

private:
	UPrimitiveComponent*	associatedComponent = nullptr;
	bool					bHasIdentity = true;

	bool									basicSimulatedPhysicsValue = false;
	FName									basicProfilName;
	TEnumAsByte<ECollisionEnabled::Type>	basicCollisionEnabledType;
};