#include "Lab/GA_Lab_WrongGiveAbility.h"

#include "Lab/AuraLabLog.h"

UGA_Lab_WrongGiveAbility::UGA_Lab_WrongGiveAbility()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UGA_Lab_WrongGiveAbility::ActivateAbility_Lab(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	AURA_LAB_LOG(Warning, TEXT("GA_Lab_WrongGiveAbility: marker ability — use AuraLab.SimulateWrongClientGiveAbility on Client for N04"));
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
