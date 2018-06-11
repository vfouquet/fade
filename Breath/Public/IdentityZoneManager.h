// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/DecalComponent.h"
#include "Containers/Array.h"
#include "ChemicalComponent.h"
#include "InteractableComponent.h"
#include "IdentityPhysicsOverrideComponent.h"
#include "IdentityEraserComponent.h"
#include "MemoryZoneComponent.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IdentityZoneManager.generated.h"

UCLASS()
class BREATH_API AIdentityZoneManager : public AActor
{
	GENERATED_BODY()

public:	
	struct FErasedObjectProperties
	{
		TWeakObjectPtr<UPrimitiveComponent>					primitiveComponent = nullptr;
		TWeakObjectPtr<UChemicalComponent>					chemicalComponent = nullptr;
		TWeakObjectPtr<UInteractableComponent>				interactableComponent = nullptr;
		TWeakObjectPtr<UIdentityPhysicsOverrideComponent>	physicsOverrider = nullptr;
		FVector								initialVelocity;
		EChemicalState						previousChemicalState;
		float								currentDecelerationTime = 0.0f;
		float								maxDeceleratingTime = 0.1f;
		bool								bWasSimulatingPhysics = false;
		bool								bDecelerating = false;
		int8								memoryZoneNbr = 0;
		int8								erasedZoneNbr = 0;
	};

	// Sets default values for this actor's properties
	AIdentityZoneManager();
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	int		AddErasedZone(UIdentityEraserComponent* value) { return erasedZones.Add(value); }
	void	RemoveZone(UIdentityEraserComponent* value);
	int		AddMemoryZone(UMemoryZoneComponent* value) { return memoryZones.Add(value); }
	void	RemoveZone(UMemoryZoneComponent* value);
	void	RemoveAffectedObject(int id) { affectedObjects.RemoveAt(id); }

	FErasedObjectProperties&	createNewProperties(UPrimitiveComponent* primitiveComponent, float decelerationTime = 0.0f);
	FErasedObjectProperties*	containsErasedObjectProperties(UPrimitiveComponent* reference, bool& foundSomething, int& outID);
	void						updateObjectProperties(FErasedObjectProperties& properties);
	void						updateObjectProperties(FErasedObjectProperties& properties, float decelerationTime);
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shader")
	UMaterialInterface*	materialInterface = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shader")
	UMaterialInterface*	DecalRoughnessMaterialInterface = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shader")
	UMaterialInterface*		DecalNormalMaterialInterface = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shader")
	UMaterialParameterCollection*	StopParametersCollection = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shader")
	float	ZoneOffset = 100.0f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	void	createNormalDecal(FVector decalExtent);
	void	createRoughnessDecal(FVector decalExtent);
	bool	computeDecalsExtent(FVector& newLocation, FVector& newExtent);

private:
	AActor*												character = nullptr;
	TArray<FErasedObjectProperties>						affectedObjects;
	TArray<TWeakObjectPtr<UIdentityEraserComponent>>	erasedZones;
	TArray<TWeakObjectPtr<UMemoryZoneComponent>>		memoryZones;
	UMaterialInstanceDynamic*							whiteZoneMaterial = nullptr;
	UMaterialInstanceDynamic*							decalRoughnessMaterial = nullptr;
	UMaterialInstanceDynamic*							decalNormalMaterial = nullptr;
	UDecalComponent*									roughnessDecal = nullptr;
	UDecalComponent*									normalDecal = nullptr;
};
