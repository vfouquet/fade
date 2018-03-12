// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TextFadeActor.generated.h"

class ATextRenderActor;
class UTextRenderComponent;

UCLASS()
class BREATH_API ATextFadeActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATextFadeActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void	Tick(float DeltaTime) override;
	virtual void	NotifyActorBeginOverlap(AActor* OtherActor) override;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text")
	ATextRenderActor*	textActor = nullptr;
	//UPROPERTY(EditAnywhere, Category = "Text")
	//FComponentReference	textComponentReference;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
	float	FadeInTime = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
	float	FadeOutTime = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
	float	WaitTime = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	bool	DoFadeIn = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	bool	DoFadeOut = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	bool	SelfDestroyAfterTrigger = true;

private:
	UFUNCTION()
	void	endFadeIn();
	UFUNCTION()
	void	endFadeOut();
	UFUNCTION()
	void	beginFadeOut();

private:
	UTextRenderComponent*	textComponent;
	FTimerHandle			timerHandle;
	bool					fadingIn = false;
	bool					fadingOut = false;
};
