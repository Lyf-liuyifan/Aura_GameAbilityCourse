// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Interaction/CombatInterface.h"
#include "AuraCharacterBase.generated.h"

class UAbilitySystemComponent;
class UAbilitySystemComponent;
class UAttributeSet;
class UGameplayEffect;

UCLASS(Abstract)
class AURA_API AAuraCharacterBase : public ACharacter, public IAbilitySystemInterface, public ICombatInterface
{
	GENERATED_BODY()

public:
	/*
	* 对人物进行初始化设置，比如人物的骨骼，武器属性的初始化（在堆上创建，使属性中指针指向它），包括碰撞预设等
	*/
	AAuraCharacterBase();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UAttributeSet* GetAttributeSet() const { return AttributeSet; }

	UFUNCTION(BlueprintCallable)
	virtual void ShowCharacterAttribute();


protected:

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TObjectPtr<USkeletalMeshComponent> Weapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	FName WeaponTipSocketName;

	virtual FVector GetCombatSocketLocation()override;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<class UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<class UAttributeSet> AttributeSet;


	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "Attribute")
	TSubclassOf<UGameplayEffect> DefaultPrimaryAttributes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute")
	TSubclassOf<UGameplayEffect> DefaultVitalAttributes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute")
	TSubclassOf<UGameplayEffect> DefaultSecondaryAttributes;

	
	
	virtual void InitAbilityActorInfo();

	void InitializePrimaryAttributes()const;

	void ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level = 1.0f) const;

	void InitializeDefaultAbilities() const;

	void AddCharacterAbilities();
private:
	UPROPERTY(EditAnywhere, Category = "CharacterAbility")
	TArray<TSubclassOf<UGameplayAbility>> CharacterAbilities;
};

