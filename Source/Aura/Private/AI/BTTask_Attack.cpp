// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_Attack.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

void UBTTask_Attack::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	// 把 BlackboardKey_Target 选择器与行为树 Blackboard 资产里的 Key 绑定，Details 面板下拉才能列出可选 Key
	UBlackboardData* BBAsset = Asset.GetBlackboardAsset();
	if (ensure(BBAsset))
	{
		BlackboardKey_Target.ResolveSelectedKey(*BBAsset);
	}
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 1. 取 AI 控制的 Pawn（即敌人本体），无效则本次攻击任务失败
	AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* OwnerPawn = AIController ? AIController->GetPawn() : nullptr;
	if (!AIController || !OwnerPawn) return EBTNodeResult::Failed;

	// 2. 从黑板取出当前攻击目标 Actor（玩家），由 BTService_FindNearestPlayer 写入
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	AActor* TargetActor = Blackboard ? Cast<AActor>(Blackboard->GetValueAsObject(BlackboardKey_Target.SelectedKeyName)) : nullptr;
	if (!Blackboard || !TargetActor) return EBTNodeResult::Failed;

	// 3. 校验 EventTag 有效，避免发空事件导致 GA 永远等不到
	if (!EventTag.IsValid()) return EBTNodeResult::Failed;

	// 4. 校验敌人身上确实有 ASC，否则 SendGameplayEventToActor 内部会静默失败
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerPawn);
	if (!ASC) return EBTNodeResult::Failed;

	// 5. 构造 GameplayEventData：Instigator=敌人自己，Target=玩家
	//    直接把目标 Actor 放在 EventData.Target，GA 侧用 Get Target from Gameplay Event 节点即可取出
	FGameplayEventData EventData;
	EventData.Instigator = OwnerPawn;
	EventData.Target = TargetActor;
	EventData.EventTag = EventTag;

	// 6. 通过 ASC 发送 GameplayEvent；GA 那边 AbilityTriggers 配了同一 Tag 就会被自动激活
	//    注意 UE5.3 该函数返回 void，无法用返回值判断成功，因此前面已显式校验 ASC / Target / Tag
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerPawn, EventTag, EventData);

	// 纯 C++ 路径：不调用 Super，蓝图子类的 Receive Execute AI 不会触发
	// 目标传递与 GA 激活全部由 C++ 的 SendGameplayEventToActor 负责
	return EBTNodeResult::Succeeded;
}
