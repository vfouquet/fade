// Fill out your copyright notice in the Description page of Project Settings.

#include "EditorUtilities.h"

#include "Editor/UnrealEd/Public/Editor.h"


void UEditorUtilities::SelectActor(AActor* Actor)
{
	GEditor->SelectActor(Actor, true, true);
}

bool UEditorUtilities::ModifyActor(AActor* Actor, bool bAlwaysMarkDirty = true)
{
	if (Actor != nullptr)
	{
		Actor->Modify(bAlwaysMarkDirty);
		return true;
	}

	return false;
}
