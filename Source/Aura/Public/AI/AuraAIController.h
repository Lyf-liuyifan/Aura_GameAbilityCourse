// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "AuraAIController.generated.h"


class UBlackboardComponent;
class UBehaviorTreeComponent;
class UAISenseConfig_Damage;

/**
 * 敌人 AI 控制器：行为树 + Damage 感知拉仇恨。
 * 感知成功后写入与 BTService 相同的黑板 Key，由 Service 同步进战斗态。
 */
UCLASS()
class AURA_API AAuraAIController : public AAIController
{
	GENERATED_BODY()
public:
	AAuraAIController();

protected:

	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> BehaviorTreeComonent;

	/** 伤害感知配置（挂到基类已有的 PerceptionComponent 上） */
	UPROPERTY()
	TObjectPtr<UAISenseConfig_Damage> DamageConfig;

	/**
	 * 受伤拉仇恨的最大距离（厘米）。
	 * 玩家超出此距离时即使造成伤害也不写入追击目标。
	 */
	UPROPERTY(EditAnywhere, Category = "AI|Perception", meta = (ClampMin = "0"))
	float ChaseMaxRange = 1500.f;

	/** Damage / 其他感知更新时：成功则写黑板拉仇恨；过期不主动清目标（交给 BTService） */
	UFUNCTION()
	void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

};
