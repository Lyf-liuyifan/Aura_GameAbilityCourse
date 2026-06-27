#pragma once

#include "CoreMinimal.h"
#include "Lab/AuraLabGameplayAbility.h"
#include "GA_Lab_MetaDamage.generated.h"

class UGameplayEffect;
class AAuraLabTargetDummy;

UCLASS()
class AURA_API UGA_Lab_MetaDamage : public UAuraLabGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Lab_MetaDamage();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Lab")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "Lab")
	float LabDamageMagnitude = 50.f;

	virtual void ActivateAbility_Lab(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	AAuraLabTargetDummy* FindNearestDummy(const AActor* SourceActor) const;
};
