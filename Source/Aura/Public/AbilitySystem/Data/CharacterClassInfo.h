// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "ScalableFloat.h"
#include "CharacterClassInfo.generated.h"

class UGameplayEffect;


//角色职业类别
UENUM(BlueprintType)
enum class ECharacterClass : uint8
{
	None,
	Elementalist,
	Warrior,
	Ranger,
};

// 角色种类：这个角色具体是哪种生物（主角 / 哪种哥布林…），决定用哪套蒙太奇；与「职业 ECharacterClass」是两个维度
UENUM(BlueprintType)
enum class ECharacterCategory : uint8
{
	Aura,
	GolbianSpear,
	GolbianShoot,
	Monster,
	GoblinShaman
};

//这是每个Character得包含的默认信息
USTRUCT(BlueprintType)
struct FCharacterClassDefaultInfo
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Class Defaults")
	TSubclassOf<UGameplayEffect> PrimaryAttributeInitEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Class Defaults")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;


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

	/** 按怪物种类索引的经验奖励：Value = 该种基础经验，Curve = 等级缩放 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "XP")
	TMap<ECharacterCategory, FScalableFloat> XPRewardMap;
	
	/** 发放经验用的 Instant GE（Modifier: XP Add, SetByCaller Data.XP） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "XP")
	TSubclassOf<UGameplayEffect> XPRewardGE;

	/** 升到下一级所需经验：GetValueAtLevel(当前玩家等级) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "XP")
	FScalableFloat PlayerLevelUpRequirement;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common Class Defaults")
	TSubclassOf<UGameplayEffect> SecondaryAttributeInitEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common Class Defaults")
	TSubclassOf<UGameplayEffect> VitalAttributeInitEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Common Class Defaults")
	TArray<TSubclassOf<UGameplayAbility>> CommonAbilities;

	FCharacterClassDefaultInfo GetDefaultInfoForClass(ECharacterClass CharacterClass) const;

	/** 查询指定种类在指定等级下的经验奖励 */
	float GetXPReward(ECharacterCategory Category, int32 Level) const;

	float GetXPForNextLevel(int32 PlayerLevel) const
	{
		return PlayerLevelUpRequirement.GetValueAtLevel(PlayerLevel);
	}
};
