// Fill out your copyright notice in the Description page of Project Settings.

#include "TextFadeActor.h"

#include "Components/TextRenderComponent.h"
#include "Engine/TextRenderActor.h"
#include "TimerManager.h"

// Sets default values
ATextFadeActor::ATextFadeActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATextFadeActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATextFadeActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (!textComponent)
		return;

	if (fadingIn)
	{
		FColor tempCol = textComponent->TextRenderColor;
		tempCol.A = FMath::Lerp(0, 255, GetWorldTimerManager().GetTimerElapsed(timerHandle) / FadeInTime);
		textComponent->SetTextRenderColor(tempCol);
		return;
	}

	if (fadingOut)
	{
		FColor	tempCol = textComponent->TextRenderColor;
		tempCol.A = FMath::Lerp(255, 0, GetWorldTimerManager().GetTimerElapsed(timerHandle) / FadeOutTime);
		textComponent->SetTextRenderColor(tempCol);
		return;
	}
}

void	ATextFadeActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (textActor)
		textComponent = textActor->GetTextRender();
	else
	{
		//if (textComponentReference.OtherActor)
		//{
		//	auto* temp = (textComponentReference.GetComponent(textComponentReference.OtherActor));
		//	textComponent = Cast<UTextRenderComponent>(textComponentReference.GetComponent(textComponentReference.OtherActor));
		//}
		if (!textComponent)
		{
			UE_LOG(LogTemp, Error, TEXT("TextFadeActor has no actor referenced"));
			if (SelfDestroyAfterTrigger)
				Destroy();
			return;
		}
	}

	if (SelfDestroyAfterTrigger)
	{
		TArray<UActorComponent*>	primComps = GetComponentsByClass(UPrimitiveComponent::StaticClass());
		for (auto* component : primComps)
		{
			UPrimitiveComponent* prim = Cast<UPrimitiveComponent>(component);
			if (prim)
				prim->Deactivate();
		}
	}

	if (DoFadeIn)
	{
		FTimerDelegate	del;
		del.BindUFunction(this, "endFadeIn");
		GetWorldTimerManager().SetTimer(timerHandle, del, FadeInTime, false);
		fadingIn = true;
		return;
	}
	if (DoFadeOut)
	{
		FTimerDelegate	del;
		del.BindUFunction(this, "endFadeOut");
		GetWorldTimerManager().SetTimer(timerHandle, del, FadeOutTime, false);
		fadingOut = true;
		return;
	}
	Destroy();
}

void	ATextFadeActor::endFadeIn()
{
	if (!DoFadeOut)
	{
		Destroy();
		return;
	}
	fadingIn = false;

	FTimerDelegate	del;
	del.BindUFunction(this, "beginFadeOut");
	GetWorldTimerManager().SetTimer(timerHandle, del, WaitTime, false);
}

void	ATextFadeActor::endFadeOut()
{
	if (SelfDestroyAfterTrigger)
		Destroy();
}

void	ATextFadeActor::beginFadeOut()
{
	FTimerDelegate	del;
	del.BindUFunction(this, "endFadeOut");
	GetWorldTimerManager().SetTimer(timerHandle, del, FadeOutTime, false);
	fadingOut = true;
}