#pragma once

#include "CoreMinimal.h"
#include "Character/AuraCharacterBase.h"
#include "Lab/LabNetSerializeTypes.h"
#include "AuraLabTargetDummy.generated.h"

/**
 * Lab 靶子：用于属性复制、RPC、NetSerialize 等网络实验。
 */
UCLASS()
class AURA_API AAuraLabTargetDummy : public AAuraCharacterBase
{
	GENERATED_BODY()

public:
	AAuraLabTargetDummy();

	virtual int32 GetPlayerLevel() override { return Level; }

	/** Server 侧递增受击计数并触发 Multicast 特效 */
	void ApplyServerHit();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayHitFX();

	/** 供 NetSerialize 实验：Server 写入后复制到 Client */
	void SetLabPayload(const FLabNetSerializePayload& NewPayload);

protected:
	virtual void BeginPlay() override;
	virtual void InitAbilityActorInfo() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_LabHitCount();

	UFUNCTION()
	void OnRep_LabPayload();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lab")
	int32 Level = 1;

	/** 受击次数，演示属性复制 */
	UPROPERTY(ReplicatedUsing = OnRep_LabHitCount, BlueprintReadOnly, Category = "Lab|Network")
	int32 LabHitCount = 0;

	/** 自定义 NetSerialize 载荷，演示结构体网络序列化 */
	UPROPERTY(ReplicatedUsing = OnRep_LabPayload, BlueprintReadOnly, Category = "Lab|Network")
	FLabNetSerializePayload LabPayload;
};
