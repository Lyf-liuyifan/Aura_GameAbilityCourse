// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "AuraGameplayAbility.generated.h"

class UAbilitySystemComponent;
class UGameplayEffect;

/**
 * 
 */
UCLASS()
class AURA_API UAuraGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UAuraGameplayAbility();
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	FGameplayTag StarupInputTag;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Damage")
	FScalableFloat Damage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage", meta = (Categories = "Damage"))
	FGameplayTag DamageTypeTag;

protected:
	UFUNCTION(BlueprintCallable)
	FGameplayEffectSpecHandle MakeDamageEffectSpec(
		UAbilitySystemComponent* SourceASC,
		TSubclassOf<UGameplayEffect> DamageEffectClass,
		UObject* OptionalSourceObject = nullptr) const;
};
