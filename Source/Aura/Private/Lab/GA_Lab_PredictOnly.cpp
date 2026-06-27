#include "Lab/GA_Lab_PredictOnly.h"

#include "Lab/AuraLabLog.h"

UGA_Lab_PredictOnly::UGA_Lab_PredictOnly()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UGA_Lab_PredictOnly::ActivateAbility_Lab(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	AURA_LAB_LOG(Warning, TEXT("GA_Lab_PredictOnly: no gameplay side-effects — observe Predicting then Confirmed in log"));
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
