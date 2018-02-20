// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Containers/SparseArray.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IdentityZoneManager.generated.h"

class UIdentityEraserComponent;

UCLASS()
class BREATH_API AIdentityZoneManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AIdentityZoneManager();
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	int		AddErasedZone(UIdentityEraserComponent* value) { return erasedZones.Add(value); }
	void	RemoveZone(int idx);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shader")
	UMaterialInterface*	materialInterface = nullptr;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	TSparseArray<UIdentityEraserComponent*>	erasedZones;
	UMaterialInstanceDynamic*				whiteZoneMaterial = nullptr;
};
