// Fill out your copyright notice in the Description page of Project Settings.

#include "IdentityZoneManager.h"

#include "Components/PostProcessComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "IdentityEraserComponent.h"

// Sets default values
AIdentityZoneManager::AIdentityZoneManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AIdentityZoneManager::BeginPlay()
{
	Super::BeginPlay();
	
	UPostProcessComponent*	postProcessComponent = FindComponentByClass<UPostProcessComponent>();
	if (!postProcessComponent || !materialInterface)
		return;

	whiteZoneMaterial = UMaterialInstanceDynamic::Create(materialInterface, this);
	
	postProcessComponent->Settings.AddBlendable(whiteZoneMaterial, 1.0f);
}

// Called every frame
void AIdentityZoneManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!whiteZoneMaterial)
		return;

	for (TSparseArray<UIdentityEraserComponent*>::TConstIterator 
		it = erasedZones.CreateConstIterator(); it; ++it)
	{
		FString indexStr = FString::FromInt(it.GetIndex() + 1);

		whiteZoneMaterial->SetScalarParameterValue(FName(*("Size_" + indexStr)), (*it)->GetScaledSphereRadius());
		whiteZoneMaterial->SetVectorParameterValue(FName(*("Position_" + indexStr)), (*it)->GetComponentLocation());
	}
}

void	AIdentityZoneManager::RemoveZone(int value)
{
	if (erasedZones.IsAllocated(value))
	{
		if (whiteZoneMaterial)
		{
			FString indexStr = FString::FromInt(value + 1);
			whiteZoneMaterial->SetScalarParameterValue(FName(*("Scale_" + indexStr)), 0.0f);
		}
		erasedZones.RemoveAt(value, 1);
	}
}