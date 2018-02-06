// Fill out your copyright notice in the Description page of Project Settings.

#include "ChemicalWaterComponent.h"

void UChemicalWaterComponent::InitializeComponent()
{
	Super::InitializeComponent();
	type = EChemicalType::Water;
}