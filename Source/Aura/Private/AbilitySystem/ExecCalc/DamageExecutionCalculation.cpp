// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/ExecCalc/DamageExecutionCalculation.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/AuraAbilityTypes.h"
#include "AuraGameplayTags.h"

// 声明需要在 Execution Calculation 中读取的属性；必须在 RelevantAttributesToCapture 中注册后才能捕获
struct AuraDamageStatics {

	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ArmorPenetration);
	DECLARE_ATTRIBUTE_CAPTUREDEF(BlockChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitDamage);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitResistance);

	// 伤害计算系数（13-16），集中管理便于后续调参
	static constexpr float CriticalHitMultiplier = 2.f;
	static constexpr float BlockDamageReductionPercent = 0.5f;
	static constexpr float ArmorPenetrationIgnorePercent = 0.25f;
	static constexpr float ArmorReductionPercent = 0.333f;

	AuraDamageStatics() {
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, Armor, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, ArmorPenetration, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, BlockChance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitChance, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitDamage, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitResistance, Target, false);
	}
};

static const AuraDamageStatics& DamageStatics() {
	static AuraDamageStatics DStatics;
	return DStatics;
		
}

UDamageExecutionCalculation::UDamageExecutionCalculation()
{
	// 未注册到这里的属性，AttemptCalculateCapturedAttributeMagnitude 会失败并保持默认值 0
	RelevantAttributesToCapture.Add(DamageStatics().ArmorDef);
	RelevantAttributesToCapture.Add(DamageStatics().ArmorPenetrationDef);
	RelevantAttributesToCapture.Add(DamageStatics().BlockChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitDamageDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitResistanceDef);
}

void UDamageExecutionCalculation::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{

	const UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	const UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

	const AActor* SourceAvatar = SourceASC ? SourceASC->GetAvatarActor() : nullptr;
	const AActor* TargetAvatar = TargetASC ? TargetASC->GetAvatarActor() : nullptr;

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	// 1. 基础伤害：由技能/GE 通过 SetByCaller 传入
	float Damage = 0.f;
	Damage = Spec.GetSetByCallerMagnitude(FAuraGameplayTags::GetSingletonInstance().Damage);

	// 2. 暴击（13-17）：有效暴击率 = 攻击者暴击率 - 目标暴击抗性，成功则 × 倍率并加上暴击伤害加成
	float SourceCriticalHitChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitChanceDef, EvaluationParameters, SourceCriticalHitChance);
	SourceCriticalHitChance = FMath::Max<float>(SourceCriticalHitChance, 0.f);

	float TargetCriticalHitResistance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitResistanceDef, EvaluationParameters, TargetCriticalHitResistance);
	TargetCriticalHitResistance = FMath::Max<float>(TargetCriticalHitResistance, 0.f);

	float SourceCriticalHitDamage = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitDamageDef, EvaluationParameters, SourceCriticalHitDamage);
	SourceCriticalHitDamage = FMath::Max<float>(SourceCriticalHitDamage, 0.f);

	const float EffectiveCriticalHitChance = SourceCriticalHitChance - TargetCriticalHitResistance;
	const bool bCriticalHit = FMath::RandRange(1, 100) < EffectiveCriticalHitChance;

	Damage = bCriticalHit ? DamageStatics().CriticalHitMultiplier * Damage + SourceCriticalHitDamage : Damage;

	// 3. 格挡：BlockChance 为 0~100 的百分比，Roll [1,100] 小于该值则格挡成功
	float TargetBlockChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().BlockChanceDef, EvaluationParameters, TargetBlockChance);
	TargetBlockChance = FMath::Max<float>(TargetBlockChance, 0.f);

	const bool bIsBlocked = FMath::RandRange(1, 100) < TargetBlockChance;
	Damage = bIsBlocked ? Damage * DamageStatics().BlockDamageReductionPercent : Damage;

	// 4. 护甲减伤：穿透先按比例削减目标护甲，再用有效护甲降低最终伤害
	float TargetArmor = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorDef, EvaluationParameters, TargetArmor);
	TargetArmor = FMath::Max<float>(TargetArmor, 0.f);

	float SourceArmorPenetration = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorPenetrationDef, EvaluationParameters, SourceArmorPenetration);
	SourceArmorPenetration = FMath::Max<float>(SourceArmorPenetration, 0.f);

	// 每 1 点穿透忽略目标 ArmorPenetrationIgnorePercent% 护甲（400 穿透可完全无视护甲）
	const float EffectiveArmor = TargetArmor * (100.f - SourceArmorPenetration * DamageStatics().ArmorPenetrationIgnorePercent) / 100.f;

	// 每 1 点有效护甲提供约 ArmorReductionPercent% 减伤（300 护甲约减 100% 伤害）
	Damage *= (100.f - EffectiveArmor * DamageStatics().ArmorReductionPercent) / 100.f;

	FGameplayEffectContextHandle EffectContextHandle = Spec.GetContext();
	FAuraGameplayEffectContext* AuraContext = FAuraGameplayEffectContext::ExtractEffectContext(EffectContextHandle);
	AuraContext->SetIsCriticalHit(bCriticalHit);
	AuraContext->SetIsBlockedHit(bIsBlocked);

	// 5. 写入 Meta 属性 InComingDamage，由 AttributeSet::PostGameplayEffectExecute 扣血
	const FGameplayModifierEvaluatedData EvaluatedData(UAuraAttributeSet::GetInComingDamageAttribute(), EGameplayModOp::Additive, Damage);
	OutExecutionOutput.AddOutputModifier(EvaluatedData);
}
