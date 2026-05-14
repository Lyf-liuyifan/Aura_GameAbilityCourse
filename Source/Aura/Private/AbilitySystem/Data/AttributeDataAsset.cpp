// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Data/AttributeDataAsset.h"

FAuraAttributeData UAttributeDataAsset::FindAttributeDataByTag(const FGameplayTag& AttributeTag, bool bLogNotFound) const
{

	for (auto& AttributeData : AttributeDataArray)
	{
		if(AttributeData.AttribuiteTag.MatchesTagExact(AttributeTag))
		{
			bLogNotFound = true;
			return AttributeData;
		}
	}

	if (bLogNotFound)
	{

	}else {
		UE_LOG(LogTemp, Log, TEXT("Cant find Tag : [%s] on Attribute [%s]"), *AttributeTag.ToString(), *GetNameSafe(this));
	}
	return FAuraAttributeData();
}
