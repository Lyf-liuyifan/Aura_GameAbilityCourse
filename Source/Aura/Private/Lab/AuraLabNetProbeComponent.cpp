#include "Lab/AuraLabNetProbeComponent.h"

#include "Lab/AuraLabLog.h"
#include "Lab/AuraLabTargetDummy.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"

UAuraLabNetProbeComponent::UAuraLabNetProbeComponent()
{
	SetIsReplicatedByDefault(true);
}

void UAuraLabNetProbeComponent::RequestLabHit()
{
	// 本地 Controlled Pawn 通过 Server RPC 把请求交给权威端
	Server_RequestLabHit();
}

void UAuraLabNetProbeComponent::Server_RequestLabHit_Implementation()
{
	// 仅在 Server 查找调用者附近的 Lab 假人
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn || !GetWorld())
	{
		return;
	}

	AAuraLabTargetDummy* NearestDummy = nullptr;
	float BestDistSq = MAX_FLT;
	for (TActorIterator<AAuraLabTargetDummy> It(GetWorld()); It; ++It)
	{
		const float DistSq = FVector::DistSquared(OwnerPawn->GetActorLocation(), It->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			NearestDummy = *It;
		}
	}

	if (!NearestDummy)
	{
		AURA_LAB_LOG(Warning, TEXT("Server_RequestLabHit: 场景中没有 Lab 假人"));
		return;
	}

	NearestDummy->ApplyServerHit();
}
