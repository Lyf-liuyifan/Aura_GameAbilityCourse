// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/AuraAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Character/AuraCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Damage.h"

AAuraAIController::AAuraAIController()
{
	Blackboard = CreateDefaultSubobject<UBlackboardComponent>("BlackboadComponent");
	check(Blackboard);
	BehaviorTreeComonent = CreateDefaultSubobject<UBehaviorTreeComponent>("BehaviorTreeComonent");
	check(BehaviorTreeComonent);
	BrainComponent = BehaviorTreeComonent;

	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>("PerceptionComponent");
	DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>("DamageConfig");

	PerceptionComponent->ConfigureSense(*DamageConfig);
	// 感知到目标（含受伤）时回调，用于写黑板拉仇恨
	PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AAuraAIController::OnPerceptionUpdated);
}

void AAuraAIController::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	// 与 BB_EnermyBB 一致：HasSeenPlayer（不是 bHasSeenPlayer）
	static const FName KEY_TargetToFollow(TEXT("TargetToFollow"));
	static const FName KEY_HasValidTarget(TEXT("HasValidTarget"));
	static const FName KEY_HasSeenPlayer(TEXT("HasSeenPlayer"));
	static const FName KEY_LastLocation(TEXT("LastLocation"));
	static const FName KEY_DistanceToTarget(TEXT("DistanceToTarget"));

	// 黑板未初始化、或刺激失败时直接返回（过期不主动清目标，避免打断已进入的追击）
	if (!Blackboard || !Stimulus.WasSuccessfullySensed())
	{
		return;
	}

	// 只认玩家角色，忽略其它 Actor 产生的刺激
	AAuraCharacter* Player = Cast<AAuraCharacter>(Actor);
	if (!Player)
	{
		return;
	}

	APawn* MyPawn = GetPawn();
	if (!MyPawn)
	{
		return;
	}

	// 超出追击范围则不拉仇恨（仍可被视野 Service 发现）
	const float Distance = FVector::Dist(MyPawn->GetActorLocation(), Player->GetActorLocation());
	if (Distance > ChaseMaxRange)
	{
		return;
	}

	// 写入战斗黑板；Service 读 HasSeenPlayer 同步 Memory
	Blackboard->SetValueAsObject(KEY_TargetToFollow, Player);
	Blackboard->SetValueAsBool(KEY_HasValidTarget, true);
	Blackboard->SetValueAsBool(KEY_HasSeenPlayer, true);
	Blackboard->SetValueAsVector(KEY_LastLocation, Player->GetActorLocation());
	Blackboard->SetValueAsFloat(KEY_DistanceToTarget, Distance);
}
