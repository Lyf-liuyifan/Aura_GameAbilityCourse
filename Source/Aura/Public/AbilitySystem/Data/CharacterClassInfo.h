// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "CharacterClassInfo.generated.h"

class UGameplayEffect;

UENUM(BlueprintType)
enum class ECharacterClass : uint8
{
	None,
	Elementalist,
	Warrior,
	Ranger,
};


//这是每个Character得包含的默认信息
USTRUCT(BlueprintType)
struct FCharacterClassDefaultInfo
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Class Defaults")
	TSubclassOf<UGameplayEffect> PrimaryAttributeInitEffect;

};

/**
 * 
 */
UCLASS()
class AURA_API UCharacterClassInfo : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, Category = "Character Class Defaults")
	TMap<ECharacterClass, FCharacterClassDefaultInfo> ClassDefaultInfoMap;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common Class Defaults")
	TSubclassOf<UGameplayEffect> SecondaryAttributeInitEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common Class Defaults")
	TSubclassOf<UGameplayEffect> VitalAttributeInitEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common Class Defaults")
	TArray<TSubclassOf<UGameplayAbility>> CommonAbilities;

	FCharacterClassDefaultInfo GetDefaultInfoForClass(ECharacterClass CharacterClass) const;
};
