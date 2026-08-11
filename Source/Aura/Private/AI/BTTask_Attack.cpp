// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_Attack.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"

namespace
{
	/** 检查 ASC 上是否有名称含 Attack 的 GA 正在运行 */
	bool IsAttackAbilityActive(UAbilitySystemComponent* ASC)
	{
		if (!ASC) return false;

		for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
		{
			if (!Spec.IsActive() || !Spec.Ability) continue;

			if (Spec.Ability->GetClass()->GetName().Contains(TEXT("Attack")))
			{
				return true;
			}
		}
		return false;
	}

	/** GA 未激活超时时，打印 ASC 上已 Grant 的能力及其 Trigger，便于对照 Tag 是否一致 */
	void LogGrantedAbilityTriggers(UAbilitySystemComponent* ASC, const FGameplayTag& SentEventTag, AActor* OwnerPawn)
	{
		if (!ASC) return;

		UE_LOG(LogTemp, Warning, TEXT("[BTTask_Attack] %s: Sent EventTag=%s but no Attack GA became Active. Granted abilities:"),
			*GetNameSafe(OwnerPawn), *SentEventTag.ToString());

		for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
		{
			if (!Spec.Ability) continue;

			FString TriggerTags;
			for (const FAbilityTriggerData& Trigger : Spec.Ability->AbilityTriggers)
			{
				if (Trigger.TriggerTag.IsValid())
				{
					TriggerTags += Trigger.TriggerTag.ToString() + TEXT(", ");
				}
			}

			if (TriggerTags.IsEmpty())
			{
				TriggerTags = TEXT("(none)");
			}

			UE_LOG(LogTemp, Warning, TEXT("[BTTask_Attack]   GA=%s | Active=%d | Triggers=[%s]"),
				*Spec.Ability->GetClass()->GetName(), Spec.IsActive(), *TriggerTags);
		}
	}
}

UBTTask_Attack::UBTTask_Attack()
{
	// 需要 Tick 等待 GA 结束
	bCreateNodeInstance = true;
	bNotifyTick = true;
}

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
	FBTAttackTaskMemory* TaskMemory = reinterpret_cast<FBTAttackTaskMemory*>(NodeMemory);
	TaskMemory->bWasAttackActive = false;
	TaskMemory->WaitTime = 0.f;

	// 1. 取 AI 控制的 Pawn（即敌人本体），无效则本次攻击任务失败
	AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* OwnerPawn = AIController ? AIController->GetPawn() : nullptr;
	if (!AIController || !OwnerPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BTTask_Attack] Failed: invalid AIController or Pawn"));
		return EBTNodeResult::Failed;
	}

	// 2. 从黑板取出当前攻击目标 Actor（玩家），由 BTService_FindNearestPlayer 写入
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	AActor* TargetActor = Blackboard ? Cast<AActor>(Blackboard->GetValueAsObject(BlackboardKey_Target.SelectedKeyName)) : nullptr;
	if (!Blackboard || !TargetActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BTTask_Attack] %s: Failed — Blackboard=%d TargetKey=%s Target=%s"),
			*GetNameSafe(OwnerPawn), Blackboard != nullptr,
			*BlackboardKey_Target.SelectedKeyName.ToString(), *GetNameSafe(TargetActor));
		return EBTNodeResult::Failed;
	}

	// 3. 校验 EventTag 有效，避免发空事件导致 GA 永远等不到
	if (!EventTag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[BTTask_Attack] %s: Failed — EventTag is invalid"), *GetNameSafe(OwnerPawn));
		return EBTNodeResult::Failed;
	}

	// 4. 校验敌人身上确实有 ASC，否则 SendGameplayEventToActor 内部会静默失败
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerPawn);
	if (!ASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BTTask_Attack] %s: Failed — no ASC on pawn"), *GetNameSafe(OwnerPawn));
		return EBTNodeResult::Failed;
	}

	// 5. 若攻击 GA 已在运行，不重复发事件（避免蒙太奇每帧重启、Launch Notify 永远到不了）
	if (IsAttackAbilityActive(ASC))
	{
		UE_LOG(LogTemp, Log, TEXT("[BTTask_Attack] %s: Attack GA already active, waiting (EventTag=%s)"),
			*GetNameSafe(OwnerPawn), *EventTag.ToString());
		TaskMemory->bWasAttackActive = true;
		return EBTNodeResult::InProgress;
	}

	// 6. 构造 GameplayEventData：Instigator=敌人自己，Target=玩家
	FGameplayEventData EventData;
	EventData.Instigator = OwnerPawn;
	EventData.Target = TargetActor;
	EventData.EventTag = EventTag;

	// 7. 通过 ASC 发送 GameplayEvent；GA 那边 AbilityTriggers 配了同一 Tag 就会被自动激活
	UE_LOG(LogTemp, Log, TEXT("[BTTask_Attack] %s: SendGameplayEvent Tag=%s Target=%s"),
		*GetNameSafe(OwnerPawn), *EventTag.ToString(), *GetNameSafe(TargetActor));
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerPawn, EventTag, EventData);

	// 等待 GA 播完蒙太奇后再 Succeeded
	return EBTNodeResult::InProgress;
}

void UBTTask_Attack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	FBTAttackTaskMemory* TaskMemory = reinterpret_cast<FBTAttackTaskMemory*>(NodeMemory);
	AAIController* AIController = OwnerComp.GetAIOwner();
	APawn* OwnerPawn = AIController ? AIController->GetPawn() : nullptr;
	UAbilitySystemComponent* ASC = OwnerPawn
		? UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerPawn)
		: nullptr;

	if (!ASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BTTask_Attack] %s: Tick Failed — ASC lost"), *GetNameSafe(OwnerPawn));
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	TaskMemory->WaitTime += DeltaSeconds;

	const bool bAttackActive = IsAttackAbilityActive(ASC);
	if (bAttackActive)
	{
		TaskMemory->bWasAttackActive = true;

		// 攻击 GA 被 HitReact 打断后若未正确 EndAbility，会永久 Active（握石不放）。
		// 超时强制取消，让 BT 能切到 MoveTo / Advance。
		if (TaskMemory->WaitTime > 3.f)
		{
			UE_LOG(LogTemp, Warning, TEXT("[BTTask_Attack] %s: Attack GA stuck Active >3s, force cancel (EventTag=%s)"),
				*GetNameSafe(OwnerPawn), *EventTag.ToString());
			FGameplayTagContainer AttackTags;
			AttackTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Abilities.Attack"), false));
			ASC->CancelAbilities(&AttackTags, nullptr, nullptr);
			for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
			{
				if (Spec.IsActive() && Spec.Ability
					&& Spec.Ability->GetClass()->GetName().Contains(TEXT("Attack")))
				{
					ASC->CancelAbilityHandle(Spec.Handle);
				}
			}
			FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		}
		return;
	}

	// 曾经 Active 过、现在不 Active = 本次攻击结束
	if (TaskMemory->bWasAttackActive)
	{
		UE_LOG(LogTemp, Log, TEXT("[BTTask_Attack] %s: Attack GA finished (EventTag=%s, Wait=%.2fs)"),
			*GetNameSafe(OwnerPawn), *EventTag.ToString(), TaskMemory->WaitTime);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// 超时兜底：GA 若未进入 Active 状态（例如触发失败），避免 BT 永久卡在 InProgress
	if (TaskMemory->WaitTime > 1.5f)
	{
		LogGrantedAbilityTriggers(ASC, EventTag, OwnerPawn);
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
	}
}
