// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UMGController.h"

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UIWidgetControllerSupported.generated.h"

/**
 * 
 */
UCLASS()
class BREATH_API UUIWidgetControllerSupported : public UUserWidget, public IUMGController
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Input")
	void			MoveUp();
	virtual void	MoveUp_Implementation() override { }
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Input")
	void			MoveDown();
	virtual void	MoveDown_Implementation() override { UE_LOG(LogTemp, Warning, TEXT("UP")); }
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Input")
	void			MoveLeft();
	virtual void	MoveLeft_Implementation() override {}
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Input")
	void			MoveRight();
	virtual void	MoveRight_Implementation() override { UE_LOG(LogTemp, Warning, TEXT("UP")); }
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Input")
	void			Validate(bool pressed = false);
	virtual void	Validate_Implementation(bool pressed = false) override {}
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Input")
	void			Cancel(bool pressed = false);
	virtual void	Cancel_Implementation(bool pressed = false) override {}
};
