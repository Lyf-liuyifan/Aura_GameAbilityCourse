// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 *  All GameplayTags
 *  Singleton containing Gameplay Tag
 */

struct FAuraGameplayTags
{
public:
	static const FAuraGameplayTags& GetSingletonInstance();
	static void InitializeNativeGameplayTags();


	/*	Vital Attribute Tags	*/
	FGameplayTag Attribute_Vital_Health;
	FGameplayTag Attribute_Vital_Mana;
	

	/*	Primary Attribute Tags	*/
	FGameplayTag Attribute_Primary_Strength;
	FGameplayTag Attribute_Primary_Intelligence;
	FGameplayTag Attribute_Primary_Resilience;
	FGameplayTag Attribute_Primary_Vigor;


	/*	Secondary Attribute Tags	*/
	FGameplayTag Attribute_Secondary_Armor;
	FGameplayTag Attribute_Secondary_MaxMana;
	FGameplayTag Attribute_Secondary_MaxHealth;


	/*	Input Tag	*/
	FGameplayTag InputTag_LMB;
	FGameplayTag InputTag_RMB;
	FGameplayTag InputTag_1;
	FGameplayTag InputTag_2;
	FGameplayTag InputTag_3;
	FGameplayTag InputTag_4;
	

	

protected:

private:
	static FAuraGameplayTags GameplayTags;
};
