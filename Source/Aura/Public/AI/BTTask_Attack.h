// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlueprintBase.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "GameplayTagContainer.h"
#include "BTTask_Attack.generated.h"

/**
 * 敌人攻击任务：从黑板取出目标 Actor，通过 GameplayEvent 把目标传给敌人自己的 ASC。
 * 发送事件后返回 InProgress，直到攻击 GA 结束再 Succeeded，避免每帧重复触发导致蒙太奇被打断。
 */
UCLASS()
class AURA_API UBTTask_Attack : public UBTTask_BlueprintBase
{
	GENERATED_BODY()

	struct FBTAttackTaskMemory
	{
		/** 是否曾检测到攻击 GA 处于 Active（用于区分「尚未激活」与「已结束」） */
		bool bWasAttackActive = false;

		/** 等待 GA 结束的累计时间，用于超时兜底 */
		float WaitTime = 0.f;
	};

public:
	UBTTask_Attack();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTAttackTaskMemory); }

	/** 行为树资产加载后解析黑板 Key 选择器，确保 SelectedKeyName 与 Blackboard 资产里的 Key 绑定 */
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

	/** 行为树节点实例里选「目标 Actor」Key（由 BTService_FindNearestPlayer 写入的 TargetToFollow） */
	UPROPERTY(EditAnywhere, Category = "Blackboard", meta = (AllowedTypes = "Object"))
	FBlackboardKeySelector BlackboardKey_Target;

	/** 触发攻击的 GameplayEvent Tag，GA 侧 AbilityTriggers 配置同一个 Tag */
	UPROPERTY(EditAnywhere, Category = "GameplayEvent")
	FGameplayTag EventTag;
};
