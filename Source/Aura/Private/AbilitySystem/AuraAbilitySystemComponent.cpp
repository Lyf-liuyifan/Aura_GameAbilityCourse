// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AuraAbilitySystemComponent.h"


//当被调用的时候说明AbilityActorInfo已经被设置好了，可以在这里做一些额外的初始化工作，比如注册一些回调函数
void UAuraAbilitySystemComponent::AbilityActorInfoSet()
{
	//绑定回调
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &UAuraAbilitySystemComponent::EffectApplied);
}

void UAuraAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>> CharacterAbilities)
{
	for(auto CharacterAbility: CharacterAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(CharacterAbility, 1);
		GiveAbilityAndActivateOnce(AbilitySpec);
	}
}

void UAuraAbilitySystemComponent::EffectApplied(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle)
{
	FGameplayTagContainer TagContainer;
	EffectSpec.GetAllAssetTags(TagContainer);
	EffectAssetTags.Broadcast(TagContainer);
	
}
