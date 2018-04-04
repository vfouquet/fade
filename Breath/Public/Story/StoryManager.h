// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "StoryManager.generated.h"

class UStoryChapter;
class UMyDataTable;

/** Structure that defines a level up table entry */
USTRUCT(BlueprintType)
struct FStoryOrderData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:

	FStoryOrderData()
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UStoryChapter*	Chapter;

};

UCLASS()
class BREATH_API AStoryManager : public AActor
{
	GENERATED_BODY()

public:	
	UMyDataTable* DataTable;

	// Sets default values for this actor's properties
	AStoryManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
