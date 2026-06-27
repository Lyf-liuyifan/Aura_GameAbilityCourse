#pragma once

#include "CoreMinimal.h"
#include "Lab/AuraLabGameplayAbility.h"
#include "GA_Lab_TargetDataProbe.generated.h"

UCLASS()
class AURA_API UGA_Lab_TargetDataProbe : public UAuraLabGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Lab_TargetDataProbe();

protected:
	virtual void ActivateAbility_Lab(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UFUNCTION()
	void OnTargetDataReceived(const FGameplayAbilityTargetDataHandle& DataHandle);
};
