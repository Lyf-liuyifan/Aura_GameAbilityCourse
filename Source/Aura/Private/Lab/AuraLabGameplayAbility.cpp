#include "Lab/AuraLabGameplayAbility.h"

#include "Lab/AuraLabLibrary.h"
#include "Lab/AuraLabLog.h"

UAuraLabGameplayAbility::UAuraLabGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UAuraLabGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	UAuraLabLibrary::LogActivationInfo(GetAvatarActorFromActorInfo(), ActivationInfo, GetClass()->GetName());
	ActivateAbility_Lab(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UAuraLabGameplayAbility::ActivateAbility_Lab(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	AURA_LAB_LOG(Log, TEXT("%s: override ActivateAbility_Lab for lab logic"), *GetClass()->GetName());
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
