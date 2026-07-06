// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlueprintBase.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "GameplayTagContainer.h"
#include "BTTask_Attack.generated.h"

/**
 * 敌人攻击任务：从黑板取出目标 Actor，通过 GameplayEvent 把目标传给敌人自己的 ASC。
 * 对应的攻击 GA（GA_MeleeAttack 等）在 AbilityTriggers 里配置同一个 EventTag，
 * 收到事件后自动激活，并用 Get Gameplay Event Data 节点从 TargetData 取出目标。
 *
 * 这样 GA 不直接依赖 Blackboard，近战 / 远程敌人可复用同一套攻击 GA。
 */
UCLASS()
class AURA_API UBTTask_Attack : public UBTTask_BlueprintBase
{
	GENERATED_BODY()
public:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	/** 行为树资产加载后解析黑板 Key 选择器，确保 SelectedKeyName 与 Blackboard 资产里的 Key 绑定 */
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

	/** 行为树节点实例里选「目标 Actor」Key（由 BTService_FindNearestPlayer 写入的 TargetToFollow） */
	UPROPERTY(EditAnywhere, Category = "Blackboard", meta = (AllowedTypes = "Object"))
	FBlackboardKeySelector BlackboardKey_Target;

	/** 触发攻击的 GameplayEvent Tag，GA 侧 AbilityTriggers 配置同一个 Tag */
	UPROPERTY(EditAnywhere, Category = "GameplayEvent")
	FGameplayTag EventTag;
};
