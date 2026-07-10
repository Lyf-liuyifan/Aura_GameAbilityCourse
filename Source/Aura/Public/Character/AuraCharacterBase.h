// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Interaction/CombatInterface.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "AuraCharacterBase.generated.h"

class UAbilitySystemComponent;
class UAbilitySystemComponent;
class UAttributeSet;
class UGameplayEffect;
class UWidgetComponent;
class UCharacterClassInfo;
class UAuraTagToMontage;

UENUM(BlueprintType)
enum class ECharacterCategory : uint8
{
	Aura,
	GolbianSpear,
	GolbianShoot,
	Monster
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttributeChangedDelegate, float, NewValue);


UCLASS(Abstract)
class AURA_API AAuraCharacterBase : public ACharacter, public IAbilitySystemInterface, public ICombatInterface
{
	GENERATED_BODY()

public:

	/* CharacterInfo */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CharacterInfo")
	ECharacterCategory CharacterCategory;

	/*
	* 对人物进行初始化设置，比如人物的骨骼，武器属性的初始化（在堆上创建，使属性中指针指向它），包括碰撞预设等
	*/
	AAuraCharacterBase();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UAttributeSet* GetAttributeSet() const { return AttributeSet; }

	UFUNCTION(BlueprintCallable)
	virtual void ShowCharacterAttribute();

	
	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastOnDeath();


	/* Combat Interface */
	virtual FVector GetCombatSocketLocation_Implementation(const FGameplayTag& SocketTag) override;
	virtual bool IsDead_Implementation() const override;
	virtual AActor* GetAvatar_Implementation()override;
	virtual void Die() override;

	/** 获取武器骨骼网格（弹弓等），供远程攻击 Attach 石头用 */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	USkeletalMeshComponent* GetWeaponMesh() const { return Weapon; }

	/** 获取 Tag -> Montage 映射数据资产 */
	UFUNCTION(BlueprintCallable, Category = "Combat|Montage")
	UAuraTagToMontage* GetTagToMontageData() const { return TagToMontageData; }

	bool bDead = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float DeathLifeSpan = 5.f;

	/* End Combat Interface */



	//本身有技能组件挂载在角色上，所以UI只需要从角色身上获得属性变化即可，本身作为一个UI Controller,再广播给UI组件
	//创造一个委托用于广播属性变化事件，UI组件绑定这个委托，当属性变化时，UI组件就会收到通知，更新UI显示
	/* UI Interface */

	virtual void BindAttributeChangeDelegate();

	
	virtual void OnHealthChanged(const FOnAttributeChangeData& Data);

	virtual void OnMaxHealthChanged(const FOnAttributeChangeData& Data);
	/* End Camera Interface */




	/* UI Properties*/
	UPROPERTY(BlueprintAssignable, Category = "UI Delegate")
	FOnAttributeChangedDelegate OnHealthChangedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "UI Delegate")
	FOnAttributeChangedDelegate OnMaxHealthChangedDelegate;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "UI Widget")
	TObjectPtr<UWidgetComponent> HealthBar;


	/* End UI Properties */




	/* Animation Interface */

	virtual void GetHitByFire();


	/* Animation Properties */

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Hit")
	TObjectPtr<UAnimMontage> HitReactMontage;


	UAnimMontage* GetHitReactAnimationMontage_Implementation();

	/* End Animation Interface */
	

	/* Material Interface */

	void Dissolve();

	UFUNCTION(BlueprintImplementableEvent)
	void StartDissolveTimeline(UMaterialInstanceDynamic* DynamicMaterial);


	UFUNCTION(BlueprintImplementableEvent)
	void StartWeaponDissolveTimeline(UMaterialInstanceDynamic* DynamicMaterial);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Material")
	TObjectPtr<UMaterialInstance> DissolveMaterialInstance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Material")
	TObjectPtr<UMaterialInstance> WeaponDissolveMaterialInstance;

	

protected:

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TObjectPtr<USkeletalMeshComponent> Weapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	FName WeaponTipSocketName;

	/* 角色攻击蒙太奇的「Tag -> Montage」映射数据资产，在角色蓝图的 CDO 中指定 DA_TagToMontage。
	*  运行时由技能/动画逻辑调用 UAuraTagToMontage::GetMontageByTagFromData 查询具体蒙太奇。
	*  DataAsset 是共享资产，多个角色实例指向同一份 .uasset 不会重复加载数据。*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Montage")
	TObjectPtr<UAuraTagToMontage> TagToMontageData;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<class UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UAttributeSet> AttributeSet;


	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "Attribute")
	TSubclassOf<UGameplayEffect> DefaultPrimaryAttributes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute")
	TSubclassOf<UGameplayEffect> DefaultVitalAttributes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attribute")
	TSubclassOf<UGameplayEffect> DefaultSecondaryAttributes;

	
	
	virtual void InitAbilityActorInfo();

	void InitializePrimaryAttributes()const;

	void ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level = 1.0f) const;

	void InitializeDefaultAttributes() const;

	void AddCharacterAbilities();



	/* CharacterClassInfo Interface */

	void InitAttributeByCharacterInfo();

	UPROPERTY(EditDefaultsOnly, Category = "CharacterInfo")
	TObjectPtr<UCharacterClassInfo>  AttributeInitDataAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterInfo")
	ECharacterClass CharacterType = ECharacterClass::None;

	/* CharacterClassInfo Interface */

private:
	UPROPERTY(EditAnywhere, Category = "CharacterAbility")
	TArray<TSubclassOf<UGameplayAbility>> CharacterAbilities;
};

