// Fill out your copyright notice in the Description page of Project Settings.

#include "StoryChapter.h"

#include "Engine/World.h"
#include "Engine/Level.h"

#include "MainPlayerStart.h"

#if WITH_EDITOR
void UStoryChapter::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetPropertyName() == "Spawn")
	{
		if (this->Spawn.IsValid())
		{
			FString LevelName;
			LevelName = this->Spawn->GetLevel()->OwningWorld->GetName();
			this->LevelName = LevelName;
		}
		else
		{
			this->LevelName = "";
		}
	}
}
#endif