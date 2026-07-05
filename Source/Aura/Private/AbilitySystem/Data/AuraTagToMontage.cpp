// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Data/AuraTagToMontage.h"



UAnimMontage* UAuraTagToMontage::GetMontageByTagFromData(AAuraCharacterBase* Character, FGameplayTag MontageTag)
{
	// 校验角色指针，空则警告并返回空指针
	if (!Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("Character is null"));
		return nullptr;
	}

	// 取出角色种类，用于在外层数组中定位该角色对应的映射项
	const ECharacterCategory Category = Character->CharacterCategory;

	// 遍历外层数组，每个 FCharacterTagMontageMap 代表一种角色种类及其蒙太奇容器
	for (const FCharacterTagMontageMap& MapEntry : CharacterTagMontages)
	{
		// 找到角色种类匹配的条目，否则继续检查下一个
		if (MapEntry.CharacterCategory != Category)
		{
			continue;
		}

		// 在该角色的容器内遍历所有「标签 -> 蒙太奇」条目，找到标签精确匹配的项
		for (const FCharacterTagMontage& TagMontage : MapEntry.CharacterTagMontages.CharacterTagMontages)
		{
			if (TagMontage.MontageTag == MontageTag)
			{
				return TagMontage.Montage;
			}
		}

		// 已经找到对应角色种类的条目却没匹配到 Tag，没必要继续遍历外层数组
		break;
	}

	// 整个数据资产中都没找到匹配项，返回空指针，由调用方自行处理
	return nullptr;
}
