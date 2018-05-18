// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interface.h"
#include "UMGController.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UUMGController : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};

/**
 * 
 */
class BREATH_API IUMGController
{
	GENERATED_IINTERFACE_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Input")
	void	MoveUp();
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Input")
	void	MoveDown();
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Input")
	void	MoveLeft();
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Input")
	void	MoveRight();
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Input")
	void	Validate(bool pressed = false);
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Input")
	void	Cancel(bool pressed = false);
};
