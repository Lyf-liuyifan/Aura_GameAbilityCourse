// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AuraEnermy.h"

AAuraEnermy::AAuraEnermy()
{
}

void AAuraEnermy::HighlightEnermy()
{
	UE_LOG(LogTemp, Log, TEXT("Highlight Enermy"));
	bIsHighlighted = true;
}

void AAuraEnermy::UnHighlightEnermy()
{
	UE_LOG(LogTemp, Log, TEXT("UnHighlight Enermy"));
	bIsHighlighted = false;
}
