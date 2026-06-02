// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "AuraDeveloperSettings.generated.h"

/**
 * 
 */
UCLASS(Config = AuraGameSetting, DefaultConfig)
class AURA_API UAuraDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	UAuraDeveloperSettings(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	

	/** Gets the settings container name for the settings, either Project or Editor */
	virtual FName GetContainerName() const override;

	/** Gets the category for the settings, some high level grouping like, Editor, Engine, Game...etc. */
	
	virtual FName GetCategoryName() const override;

	/** The unique name for your section of settings, uses the class's FName. */
	
	virtual FName GetSectionName() const override;

	UAuraDeveloperSettings* GetAuraDeveloperSettings() const { return GetMutableDefault<UAuraDeveloperSettings>(); }


	UPROPERTY(Config, BlueprintReadWrite, EditAnywhere, Category = "Sound")
	float SoundEffectVolume = 100.f;
};
