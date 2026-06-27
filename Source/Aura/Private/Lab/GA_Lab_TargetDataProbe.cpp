#include "Lab/GA_Lab_TargetDataProbe.h"

#include "Lab/AuraLabLog.h"
#include "AbilitySystem/AbilityTasks/TargetDataUnderMouse.h"
#include "Abilities/GameplayAbilityTargetTypes.h"

UGA_Lab_TargetDataProbe::UGA_Lab_TargetDataProbe()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UGA_Lab_TargetDataProbe::ActivateAbility_Lab(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	UTargetDataUnderMouse* Task = UTargetDataUnderMouse::CreateTargetDataUnderMouse(this);
	Task->MouseTargetDataDelegate.AddDynamic(this, &UGA_Lab_TargetDataProbe::OnTargetDataReceived);
	Task->ReadyForActivation();
}

void UGA_Lab_TargetDataProbe::OnTargetDataReceived(const FGameplayAbilityTargetDataHandle& DataHandle)
{
	FVector HitLocation = FVector::ZeroVector;
	if (DataHandle.Num() > 0)
	{
		if (const FGameplayAbilityTargetData_SingleTargetHit* HitData = static_cast<const FGameplayAbilityTargetData_SingleTargetHit*>(DataHandle.Get(0)))
		{
			HitLocation = HitData->HitResult.ImpactPoint;
		}
	}

	const bool bAuth = GetAvatarActorFromActorInfo() && GetAvatarActorFromActorInfo()->HasAuthority();
	AURA_LAB_LOG(Warning, TEXT("TargetDataProbe | Hit=%s Authority=%d"), *HitLocation.ToString(), bAuth);

	if (IsActive())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}
