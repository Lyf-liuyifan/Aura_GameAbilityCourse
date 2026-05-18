// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/AuraGameplayAbility.h"


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
		if (const UAuraGameplayAbility* AuraAbility = Cast<UAuraGameplayAbility>(AbilitySpec.Ability))
		{
			//给予能力，但是能力是在编译器中创建的还有StartupInputTag也是在编译器中赋值，所以还要激活能力
			AbilitySpec.DynamicAbilityTags.AddTag(AuraAbility->StarupInputTag);
			GiveAbility(AbilitySpec);
		}
		
	}
}

//这些操作都要记得修改Ability中的Spec
void UAuraAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag)) {
			AbilitySpecInputPressed(AbilitySpec);
			if (!AbilitySpec.IsActive()) {
				TryActivateAbility(AbilitySpec.Handle);
			}
		}
	}
}

void UAuraAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag)) {
			AbilitySpecInputReleased(AbilitySpec);
			CancelAbilityHandle(AbilitySpec.Handle);
		}
	}
}

void UAuraAbilitySystemComponent::EffectApplied(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle)
{
	FGameplayTagContainer TagContainer;
	EffectSpec.GetAllAssetTags(TagContainer);
	EffectAssetTags.Broadcast(TagContainer);
	
}
