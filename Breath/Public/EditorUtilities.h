// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EditorUtilities.generated.h"

/**
 * 
 */
UCLASS()
class BREATH_API UEditorUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	UFUNCTION(BlueprintCallable)
	static void SelectActor(AActor* Actor);
	UFUNCTION(BlueprintCallable)
	static bool ModifyActor(AActor* Actor, bool bAlwaysMarkDirty);
	
};
