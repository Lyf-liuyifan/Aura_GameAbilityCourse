#include "Lab/GA_Lab_PredictKeys.h"

#include "Lab/AuraLabLog.h"
#include "AuraGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "GameplayPrediction.h"

UGA_Lab_PredictKeys::UGA_Lab_PredictKeys()
{
	// 客户端先跑、再问服务器：才能看到 Predicting → Confirmed
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	// LabInputTag 不在构造里取 NativeTag（CDO 可能早于 InitializeNativeGameplayTags）
	// 真正绑定时在 CharacterBase::AddCharacterAbilities 用运行时 Tag
}


void UGA_Lab_PredictKeys::ActivateAbility_Lab(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 1. 激活会话钥匙（整次技能）——基类已打 Mode，这里再打 Current/Base
	const FPredictionKey& ActKey = ActivationInfo.GetActivationPredictionKey();
	AURA_LAB_LOG(Warning, TEXT("PredictKeys | ActKey Current=%d Base=%d Auth=%d"),
		ActKey.Current,
		ActKey.Base,
		ActorInfo->IsNetAuthority());

	// 2. 开预测窗：RAII 生成「这一步」的依赖 Key，离开作用域自动还原
	{
		FScopedPredictionWindow ScopedWindow(ASC, true);

		const FPredictionKey& ScopedKey = ASC->ScopedPredictionKey;
		AURA_LAB_LOG(Warning, TEXT("PredictKeys | ScopedKey Current=%d Base=%d (Base 常指向 ActKey)"),
			ScopedKey.Current,
			ScopedKey.Base);

		// 真实项目里：扣蓝 / 播 Montage / 发 TargetData 都写在这个窗内
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
