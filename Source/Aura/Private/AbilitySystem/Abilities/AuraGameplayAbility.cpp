// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/AuraGameplayAbility.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/AuraAbilityTypes.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AuraGameplayTags.h"
#include "GameplayEffect.h"

UAuraGameplayAbility::UAuraGameplayAbility()
{
	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::GetSingletonInstance();
	DamageTypeTag = GameplayTags.Damage_Fire;
}

FString UAuraGameplayAbility::GetAbilityTriggerTagsDebugString() const
{
	FString Out;
	for (const FAbilityTriggerData& Trigger : AbilityTriggers)
	{
		if (Trigger.TriggerTag.IsValid())
		{
			Out += Trigger.TriggerTag.ToString() + TEXT(", ");
		}
	}
	return Out.IsEmpty() ? TEXT("(none)") : Out;
}

FGameplayEffectSpecHandle UAuraGameplayAbility::MakeDamageEffectSpec(
	UAbilitySystemComponent* SourceASC,
	TSubclassOf<UGameplayEffect> DamageEffectClass,
	UObject* OptionalSourceObject) const
{
	check(SourceASC);
	check(DamageEffectClass);

	const float AbilityLevel = GetAbilityLevel();
	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::GetSingletonInstance();

	FGameplayEffectContextHandle EffectContextHandle = SourceASC->MakeEffectContext();
	if (OptionalSourceObject)
	{
		EffectContextHandle.AddSourceObject(OptionalSourceObject);
	}

	if (FAuraGameplayEffectContext* AuraContext = FAuraGameplayEffectContext::ExtractEffectContext(EffectContextHandle))
	{
		if (DamageTypeTag.IsValid())
		{
			AuraContext->SetDamageType(DamageTypeTag);
		}
		else
		{
			ensureMsgf(false, TEXT("%s: DamageTypeTag is not set. Configure it in the ability blueprint Class Defaults."), *GetName());
		}
	}

	FGameplayEffectSpecHandle DamageSpec = SourceASC->MakeOutgoingSpec(DamageEffectClass, AbilityLevel, EffectContextHandle);
	const float DamageValue = Damage.GetValueAtLevel(AbilityLevel);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(DamageSpec, GameplayTags.Damage, DamageValue);

	return DamageSpec;
}
