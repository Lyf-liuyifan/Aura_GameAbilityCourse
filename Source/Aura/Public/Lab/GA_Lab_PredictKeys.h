#pragma once

#include "CoreMinimal.h"
#include "Lab/AuraLabGameplayAbility.h"
#include "GA_Lab_PredictKeys.generated.h"

/**
 * Lab 预测探针：LocalPredicted 激活，开 Scoped 窗并打印激活 Key / 窗内 Key。
 * 进游戏后按数字键 4 触发（由角色自动 Grant + InputTag.4）。
 */
UCLASS()
class AURA_API UGA_Lab_PredictKeys : public UAuraLabGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Lab_PredictKeys();

	/** 与 IMC / ASC DynamicAbilityTags 对齐的输入标签（默认 InputTag.4） */
	UPROPERTY(EditDefaultsOnly, Category = "Lab|Input")
	FGameplayTag LabInputTag;

protected:
	virtual void ActivateAbility_Lab(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
