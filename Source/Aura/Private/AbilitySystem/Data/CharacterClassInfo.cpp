// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Data/CharacterClassInfo.h"

FCharacterClassDefaultInfo UCharacterClassInfo::GetDefaultInfoForClass(ECharacterClass CharacterClass) const
{
	return ClassDefaultInfoMap.FindChecked(CharacterClass);
}

float UCharacterClassInfo::GetXPReward(ECharacterCategory Category, int32 Level) const
{
	return XPRewardMap.FindChecked(Category).GetValueAtLevel(Level);
}


