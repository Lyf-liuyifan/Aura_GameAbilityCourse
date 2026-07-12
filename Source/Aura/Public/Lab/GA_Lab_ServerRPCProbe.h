#pragma once

#include "CoreMinimal.h"
#include "Lab/AuraLabGameplayAbility.h"
#include "GA_Lab_ServerRPCProbe.generated.h"

/**
 * Lab 能力：Client 激活 → Server RPC → 递增假人 HitCount → Multicast 特效。
 */
UCLASS()
class AURA_API UGA_Lab_ServerRPCProbe : public UAuraLabGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Lab_ServerRPCProbe();

protected:
	virtual void ActivateAbility_Lab(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
