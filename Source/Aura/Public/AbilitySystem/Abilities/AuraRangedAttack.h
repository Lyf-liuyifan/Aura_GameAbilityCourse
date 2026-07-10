// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AuraProjectilesSpell.h"
#include "AuraRangedAttack.generated.h"

class AAuraCharacterBase;
class AAuraProjectile;

/**
 * 敌人远程攻击（弹弓）：拉弓 Notify 时在皮兜生成石头并 Attach，
 * 松手 Notify 时 Launch 直线飞向 BT 传入的目标。
 */
UCLASS()
class AURA_API UAuraRangedAttack : public UAuraProjectilesSpell
{
	GENERATED_BODY()

public:
	UAuraRangedAttack();

	virtual void PostInitProperties() override;

	/** 确保蓝图 CDO 在序列化后仍保留正确的 GameplayEvent 触发器 */
	void EnsureAbilityTriggersRegistered();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;

protected:
	/** 蒙太奇 Notify：开始拉弓，生成石头并 Attach 到皮兜 */
	UPROPERTY(EditDefaultsOnly, Category = "RangedAttack|Tags")
	FGameplayTag EventTag_RockSpawn;

	/** 蒙太奇 Notify：松手发射 */
	UPROPERTY(EditDefaultsOnly, Category = "RangedAttack|Tags")
	FGameplayTag EventTag_RockLaunch;

	/** 攻击蒙太奇 Tag，从 TagToMontage 数据资产查 */
	UPROPERTY(EditDefaultsOnly, Category = "RangedAttack|Tags")
	FGameplayTag AttackMontageTag;

	/** 石头 Attach 的 Socket（弹弓 SKM 上的 Pouch） */
	UPROPERTY(EditDefaultsOnly, Category = "RangedAttack|Socket")
	FName ProjectileAttachSocketName = FName("Pouch");

	UFUNCTION()
	void OnRockSpawnEvent(FGameplayEventData Payload);

	UFUNCTION()
	void OnRockLaunchEvent(FGameplayEventData Payload);

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();

	/** 从 Activate 的 EventData 解析攻击目标位置 */
	bool ResolveTargetLocation(const FGameplayEventData* TriggerEventData, FVector& OutLocation) const;

	/** 生成石头并挂到 Weapon 的 Pouch Socket */
	AAuraProjectile* SpawnAndAttachHeldRock(AAuraCharacterBase* Character);

	/** 当前能力实例持有的石头（两阶段之间共享） */
	UPROPERTY()
	TObjectPtr<AAuraProjectile> HeldRock;

	FVector TargetLocation = FVector::ZeroVector;
};
