#include "Lab/GA_Lab_ServerRPCProbe.h"

#include "Lab/AuraLabLog.h"
#include "Lab/AuraLabNetProbeComponent.h"

UGA_Lab_ServerRPCProbe::UGA_Lab_ServerRPCProbe()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UGA_Lab_ServerRPCProbe::ActivateAbility_Lab(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 确保 Pawn 挂有 Lab 网络探针组件
	UAuraLabNetProbeComponent* Probe = Avatar->FindComponentByClass<UAuraLabNetProbeComponent>();
	if (!Probe)
	{
		Probe = NewObject<UAuraLabNetProbeComponent>(Avatar, TEXT("AuraLabNetProbe"));
		Probe->RegisterComponent();
	}

	AURA_LAB_LOG(Warning, TEXT("ServerRPCProbe 激活 | Authority=%d"), Avatar->HasAuthority());
	Probe->RequestLabHit();

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
