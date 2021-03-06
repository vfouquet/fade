// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UMGController.h"
#include "WidgetTree.h"

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
	virtual void	MoveDown_Implementation() override { }
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Input")
	void			MoveLeft();
	virtual void	MoveLeft_Implementation() override {}
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Input")
	void			MoveRight();
	virtual void	MoveRight_Implementation() override { }
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Input")
	void			Validate(bool pressed = false);
	virtual void	Validate_Implementation(bool pressed = false) override {}
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Input")
	void			Cancel(bool pressed = false);
	virtual void	Cancel_Implementation(bool pressed = false) override {}
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Input")
	void			SpecialButton();
	virtual void	SpecialButton_Implementation() override {}
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Behaviour")
	void			OnUnpause();
	virtual void	OnUnpause_Implementation() override {}

	UFUNCTION(BlueprintCallable)
	UWidget*	FindWidgetByName(FName name) { return WidgetTree ? WidgetTree->FindWidget(name) : nullptr; }

	UFUNCTION(BlueprintCallable)
	static void	DestroyObject(UObject* object) { if (object) object->ConditionalBeginDestroy(); }
};
