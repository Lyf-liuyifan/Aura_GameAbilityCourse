// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_FindNearestPlayer.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UBTService_FindNearestPlayer : public UBTService
{
	GENERATED_BODY()

public:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	UBTService_FindNearestPlayer();

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Blackboard", meta = (AllowedTypes = "Object"))
	FBlackboardKeySelector BlackboardKeyTargetToFollow;

	/** 是否找到有效目标（可选，方便 BT Decorator 使用） */
	UPROPERTY(EditAnywhere, Category = "Blackboard", meta = (AllowedTypes = "Bool"))
	FBlackboardKeySelector BlackboardKeyHasValidTarget;

	UPROPERTY(EditAnywhere, Category = "Blackboard", meta = (AllowedTypes = "Vector"))
	FBlackboardKeySelector BlackboardKeyPatrolLocation;

	UPROPERTY(EditAnywhere, Category = "Blackboard", meta = (AllowedTypes = "Vector"))
	FBlackboardKeySelector BlackboardKeyLocationFarFromTarget;

	UPROPERTY(EditAnywhere, Category = "Blackboard", meta = (AllowedTypes = "Bool"))
	FBlackboardKeySelector BlackboardKeyIsMelee;

	UPROPERTY(EditAnywhere, Category = "Blackboard", meta = (AllowedTypes = "Bool"))
	FBlackboardKeySelector BlackboardKeybNeedsToRetreat;

	UPROPERTY(EditAnywhere, Category = "Blackboard", meta = (AllowedTypes = "Vector"))
	FBlackboardKeySelector BlackboardKeyLastLocation;



	/** 最远发现距离（厘米） */
	UPROPERTY(EditAnywhere, Category = "AI")
	float SearchRadius = 1000.f;

	/** 视野半角（度）。45 = 正前方左右各 45°，总视野 90° */
	UPROPERTY(EditAnywhere, Category = "AI", meta = (ClampMin = "0", ClampMax = "180"))
	float HalfSightAngle = 45.f;

	/** 是否做视线检测（中间有墙则看不见） */
	UPROPERTY(EditAnywhere, Category = "AI")
	bool bUseLineOfSight = true;

	/** 远程单位与玩家保持的理想距离（厘米） */
	UPROPERTY(EditAnywhere, Category = "AI", meta = (ClampMin = "0"))
	float AttackDistance = 500.f;

	FVector LastLocation;
};
