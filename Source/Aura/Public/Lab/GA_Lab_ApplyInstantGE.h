#pragma once

#include "CoreMinimal.h"
#include "Lab/AuraLabGameplayAbility.h"
#include "GA_Lab_ApplyInstantGE.generated.h"

class UGameplayEffect;

UCLASS()
class AURA_API UGA_Lab_ApplyInstantGE : public UAuraLabGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Lab_ApplyInstantGE();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Lab")
	TSubclassOf<UGameplayEffect> InstantEffectClass;

	virtual void ActivateAbility_Lab(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
