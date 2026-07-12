#include "Lab/GA_Lab_NetSerializeProbe.h"

#include "Lab/AuraLabLog.h"
#include "Lab/AuraLabTargetDummy.h"
#include "Lab/LabNetSerializeTypes.h"
#include "EngineUtils.h"

UGA_Lab_NetSerializeProbe::UGA_Lab_NetSerializeProbe()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

void UGA_Lab_NetSerializeProbe::ActivateAbility_Lab(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar || !Avatar->HasAuthority())
	{
		AURA_LAB_LOG(Warning, TEXT("NetSerializeProbe 仅在 Server 写入载荷"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	AAuraLabTargetDummy* Dummy = FindNearestDummy(Avatar);
	if (!Dummy)
	{
		AURA_LAB_LOG(Warning, TEXT("NetSerializeProbe: 未找到 Lab 假人"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 构造载荷并由假人复制到各 Client
	FLabNetSerializePayload Payload;
	Payload.BuffId = LabBuffId;
	Payload.Magnitude = LabMagnitude;
	Dummy->SetLabPayload(Payload);

	AURA_LAB_LOG(Warning, TEXT("NetSerializeProbe Server 写入 | BuffId=%d Magnitude=%.2f"), LabBuffId, LabMagnitude);
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

AAuraLabTargetDummy* UGA_Lab_NetSerializeProbe::FindNearestDummy(const AActor* SourceActor) const
{
	if (!SourceActor || !GetWorld()) return nullptr;

	AAuraLabTargetDummy* Nearest = nullptr;
	float BestDistSq = MAX_FLT;
	for (TActorIterator<AAuraLabTargetDummy> It(GetWorld()); It; ++It)
	{
		const float DistSq = FVector::DistSquared(SourceActor->GetActorLocation(), It->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Nearest = *It;
		}
	}
	return Nearest;
}
