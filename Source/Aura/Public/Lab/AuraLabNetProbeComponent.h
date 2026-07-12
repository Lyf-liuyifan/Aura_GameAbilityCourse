#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AuraLabNetProbeComponent.generated.h"

/**
 * 挂在玩家 Pawn 上，提供 Lab Server RPC 入口（Client 无法直接 RPC 非拥有的 Actor）。
 */
UCLASS(ClassGroup = (Aura), meta = (BlueprintSpawnableComponent))
class AURA_API UAuraLabNetProbeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAuraLabNetProbeComponent();

	/** Client 调用：请求 Server 对最近 Lab 假人结算受击 */
	UFUNCTION(BlueprintCallable, Category = "Aura|Lab")
	void RequestLabHit();

	UFUNCTION(Server, Reliable)
	void Server_RequestLabHit();
};
