#pragma once

#include "CoreMinimal.h"
#include "Lab/AuraLabGameplayAbility.h"
#include "GA_Lab_NetSerializeProbe.generated.h"

/**
 * Lab 能力：Server 写入 FLabNetSerializePayload，Client 通过 OnRep 观察同步结果。
 */
UCLASS()
class AURA_API UGA_Lab_NetSerializeProbe : public UAuraLabGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Lab_NetSerializeProbe();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Lab")
	uint8 LabBuffId = 42;

	UPROPERTY(EditDefaultsOnly, Category = "Lab")
	float LabMagnitude = 3.14f;

	virtual void ActivateAbility_Lab(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	class AAuraLabTargetDummy* FindNearestDummy(const AActor* SourceActor) const;
};
