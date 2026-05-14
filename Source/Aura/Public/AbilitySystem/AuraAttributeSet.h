// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectExtension.h"
#include "GameplayEffectTypes.h"
#include "AuraAttributeSet.generated.h"

/**
 * 
 */
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

//存储静态函数指针模版，用于映射标签到属性
//using AttributeFuncPtr = TBaseStaticDelegateInstance<FGameplayAttribute(), FDefaultDelegateUserPolicy>::FFuncPtr;
template<class T>
using TStaticFuncPtr = typename TBaseStaticDelegateInstance<T, FDefaultDelegateUserPolicy>::FFuncPtr;

USTRUCT(BlueprintType)
struct FEffectProperties
{
	GENERATED_BODY()

	FEffectProperties() {};

	FGameplayEffectContextHandle EffectContextHandle;

	UPROPERTY()
	UAbilitySystemComponent* SourceASC = nullptr;

	UPROPERTY()
	AActor* SourceAvatarActor = nullptr;

	UPROPERTY()
	AController* SourceController = nullptr;

	UPROPERTY()
	ACharacter* SourceCharacter = nullptr;

	UPROPERTY()
	UAbilitySystemComponent* TargetASC = nullptr;

	UPROPERTY()
	AActor* TargetAvatarActor = nullptr;

	UPROPERTY()
	AController* TargetController = nullptr;

	UPROPERTY()
	ACharacter* TargetCharacter = nullptr;
	
};

UCLASS()
class AURA_API UAuraAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
	UAuraAttributeSet();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	

	TMap<FGameplayTag, TStaticFuncPtr<FGameplayAttribute()>> TagsToAttributes;


	/*
	*	Primary Attribute Functions
	* 
	*/

	UFUNCTION()
	void OnRep_Strength(const FGameplayAttributeData& OldStrength);

	UFUNCTION()
	void OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence);

	UFUNCTION()
	void OnRep_Resilience(const FGameplayAttributeData& OldResilience);

	UFUNCTION()
	void OnRep_Vigor(const FGameplayAttributeData& OldVigor);

	/*
	*	Secondary Attribute Functions (derived via GE from Primary / other Secondary; see design chart)
	*/

	UFUNCTION()
	void OnRep_Armor(const FGameplayAttributeData& OldArmor);

	UFUNCTION()
	void OnRep_ArmorPenetration(const FGameplayAttributeData& OldArmorPenetration);

	UFUNCTION()
	void OnRep_BlockChance(const FGameplayAttributeData& OldBlockChance);

	UFUNCTION()
	void OnRep_CriticalHitChance(const FGameplayAttributeData& OldCriticalHitChance);

	UFUNCTION()
	void OnRep_CriticalHitDamage(const FGameplayAttributeData& OldCriticalHitDamage);

	UFUNCTION()
	void OnRep_CriticalHitResistance(const FGameplayAttributeData& OldCriticalHitResistance);

	UFUNCTION()
	void OnRep_HealthRegeneration(const FGameplayAttributeData& OldHealthRegeneration);

	UFUNCTION()
	void OnRep_ManaRegeneration(const FGameplayAttributeData& OldManaRegeneration);


	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);

	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana);
	/*
	*	Vital Attribute Functions Start
	*/

	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth);
	
	UFUNCTION()
	void OnRep_Mana(const FGameplayAttributeData& OldMana);


	/*
	*	Vital Attribute Functions End
	*/

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;


	/*
	*	Primary ATTRIBUTE
	*/
	UPROPERTY(BlueprintReadOnly, Category = "Primary Attributes", ReplicatedUsing = OnRep_Strength)
	FGameplayAttributeData Strength;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Strength);

	UPROPERTY(BlueprintReadOnly, Category = "Primary Attributes", ReplicatedUsing = OnRep_Intelligence)
	FGameplayAttributeData Intelligence;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Intelligence);

	UPROPERTY(BlueprintReadOnly, Category = "Primary Attributes", ReplicatedUsing = OnRep_Resilience)
	FGameplayAttributeData Resilience;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Resilience);

	UPROPERTY(BlueprintReadOnly, Category = "Primary Attributes", ReplicatedUsing = OnRep_Vigor)
	FGameplayAttributeData Vigor;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Vigor);

	/*
	*	Secondary Attributes — backing relationships are enforced in Content (Infinite GE + AttributeBased), not in C++:
	*	Armor, ArmorPenetration <- Resilience | BlockChance, CriticalHitResistance <- Armor
	*	CriticalHitChance, CriticalHitDamage <- ArmorPenetration | HealthRegeneration, MaxHealth <- Vigor (MaxHealth is Vital below)
	*	ManaRegeneration, MaxMana <- Intelligence
	*/
	UPROPERTY(BlueprintReadOnly, Category = "Secondary Attributes", ReplicatedUsing = OnRep_Armor)
	FGameplayAttributeData Armor;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Armor);

	UPROPERTY(BlueprintReadOnly, Category = "Secondary Attributes", ReplicatedUsing = OnRep_ArmorPenetration)
	FGameplayAttributeData ArmorPenetration;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, ArmorPenetration);

	UPROPERTY(BlueprintReadOnly, Category = "Secondary Attributes", ReplicatedUsing = OnRep_BlockChance)
	FGameplayAttributeData BlockChance;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, BlockChance);

	UPROPERTY(BlueprintReadOnly, Category = "Secondary Attributes", ReplicatedUsing = OnRep_CriticalHitChance)
	FGameplayAttributeData CriticalHitChance;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, CriticalHitChance);

	UPROPERTY(BlueprintReadOnly, Category = "Secondary Attributes", ReplicatedUsing = OnRep_CriticalHitDamage)
	FGameplayAttributeData CriticalHitDamage;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, CriticalHitDamage);

	UPROPERTY(BlueprintReadOnly, Category = "Secondary Attributes", ReplicatedUsing = OnRep_CriticalHitResistance)
	FGameplayAttributeData CriticalHitResistance;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, CriticalHitResistance);

	UPROPERTY(BlueprintReadOnly, Category = "Secondary Attributes", ReplicatedUsing = OnRep_HealthRegeneration)
	FGameplayAttributeData HealthRegeneration;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, HealthRegeneration);

	UPROPERTY(BlueprintReadOnly, Category = "Secondary Attributes", ReplicatedUsing = OnRep_ManaRegeneration)
	FGameplayAttributeData ManaRegeneration;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, ManaRegeneration);

	/*
		Vital Attribute
	*/
	UPROPERTY(BlueprintReadOnly, Category = "Vital Attributes", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Health);

	UPROPERTY(BlueprintReadOnly, Category = "Vital Attributes", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, MaxHealth);

	UPROPERTY(BlueprintReadOnly, Category = "Vital Attributes", ReplicatedUsing = OnRep_Mana)
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Mana);

	UPROPERTY(BlueprintReadOnly, Category = "Vital Attributes", ReplicatedUsing = OnRep_MaxMana)
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS(UAuraAttributeSet, MaxMana);

private:
	void SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& Props);
};
