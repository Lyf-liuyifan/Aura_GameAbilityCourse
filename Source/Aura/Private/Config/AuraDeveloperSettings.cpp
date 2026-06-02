// Fill out your copyright notice in the Description page of Project Settings.


#include "Config/AuraDeveloperSettings.h"

UAuraDeveloperSettings::UAuraDeveloperSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	UE_LOG(LogTemp, Log, TEXT("AuraDeveloperSettings Constructor Called"));
}

FName UAuraDeveloperSettings::GetContainerName() const
{
	FName ContainerName = FName(TEXT("Project"));
	return ContainerName;
}

FName UAuraDeveloperSettings::GetCategoryName() const
{
	return FName(TEXT("AuraGameSetting"));
}

FName UAuraDeveloperSettings::GetSectionName() const
{
	return FName(TEXT("AuraDeveloperSettings"));
}
