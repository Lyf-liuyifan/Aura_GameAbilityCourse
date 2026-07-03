// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_FindNearestPlayer.generated.h"

class AAuraCharacter;
class UNavigationSystemV1;
class UBlackboardComponent;

struct FBTFindNearestPlayerMemory
{
	bool bHasSeenPlayer = false;
};

/**
 * 感知服务：巡逻态用视野锥首次发现玩家，战斗态全图追踪 NavMesh 上的玩家。
 */
UCLASS()
class AURA_API UBTService_FindNearestPlayer : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_FindNearestPlayer();

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTFindNearestPlayerMemory); }

protected:
	UPROPERTY(EditAnywhere, Category = "Blackboard", meta = (AllowedTypes = "Object"))
	FBlackboardKeySelector BlackboardKeyTargetToFollow;

	UPROPERTY(EditAnywhere, Category = "Blackboard", meta = (AllowedTypes = "Bool"))
	FBlackboardKeySelector BlackboardKeyHasValidTarget;

	UPROPERTY(EditAnywhere, Category = "Blackboard", meta = (AllowedTypes = "Bool"))
	FBlackboardKeySelector BlackboardKeybHasSeenPlayer;

	UPROPERTY(EditAnywhere, Category = "Blackboard", meta = (AllowedTypes = "Vector"))
	FBlackboardKeySelector BlackboardKeyPatrolLocation;

	UPROPERTY(EditAnywhere, Category = "Blackboard", meta = (AllowedTypes = "Vector"))
	FBlackboardKeySelector BlackboardKeyLocationFarFromTarget;

	UPROPERTY(EditAnywhere, Category = "Blackboard", meta = (AllowedTypes = "Bool"))
	FBlackboardKeySelector BlackboardKeyIsMelee;

	UPROPERTY(EditAnywhere, Category = "Blackboard", meta = (AllowedTypes = "Bool"))
	FBlackboardKeySelector BlackboardKeybNeedsToRetreat;

	UPROPERTY(EditAnywhere, Category = "Blackboard", meta = (AllowedTypes = "Bool"))
	FBlackboardKeySelector BlackboardKeybNeedsToAdvance;

	UPROPERTY(EditAnywhere, Category = "Blackboard", meta = (AllowedTypes = "Vector"))
	FBlackboardKeySelector BlackboardKeyLastLocation;

	UPROPERTY(EditAnywhere, Category = "Blackboard", meta = (AllowedTypes = "Float"))
	FBlackboardKeySelector BlackboardKeyDistanceToTarget;

	/** 巡逻态最远发现距离（厘米） */
	UPROPERTY(EditAnywhere, Category = "AI")
	float SearchRadius = 1000.f;

	/** 视野半角（度）。45 = 正前方左右各 45°，总视野 90° */
	UPROPERTY(EditAnywhere, Category = "AI", meta = (ClampMin = "0", ClampMax = "180"))
	float HalfSightAngle = 45.f;

	/** 巡逻态是否做视线检测（中间有墙则看不见） */
	UPROPERTY(EditAnywhere, Category = "AI")
	bool bUseLineOfSight = true;

	/** 近战攻击 / MoveTo 接受半径（厘米） */
	UPROPERTY(EditAnywhere, Category = "AI", meta = (ClampMin = "0"))
	float MeleeAttackDistance = 150.f;

	/** 远程理想攻击距离（厘米） */
	UPROPERTY(EditAnywhere, Category = "AI", meta = (ClampMin = "0"))
	float RangedAttackDistance = 500.f;

	/** 玩家距敌人小于此值时远程单位后撤（厘米） */
	UPROPERTY(EditAnywhere, Category = "AI", meta = (ClampMin = "0"))
	float RetreatDistance = 400.f;

	/** 玩家距敌人大于此值时远程单位追击（厘米） */
	UPROPERTY(EditAnywhere, Category = "AI", meta = (ClampMin = "0"))
	float RangedAttackMax = 650.f;

	/** NavMesh 投影搜索范围 */
	UPROPERTY(EditAnywhere, Category = "AI", meta = (ClampMin = "0"))
	FVector NavProjectionExtent = FVector(500.f, 500.f, 500.f);

	AAuraCharacter* FindPlayerInSight(APawn* MyPawn, const FVector& MyLocation, float& OutDistance) const;
	AAuraCharacter* FindPlayerOnNavMesh(UWorld* World, const FVector& MyLocation, float& OutDistance, bool& bOutOnNavMesh) const;

	void UpdateRangedDistanceData(
		UBlackboardComponent* Blackboard,
		const FVector& MyLocation,
		const FVector& PlayerLocation,
		UNavigationSystemV1* NavSys) const;

	void ClearCombatBlackboard(UBlackboardComponent* Blackboard, FBTFindNearestPlayerMemory* Memory) const;
	void SetHasSeenPlayer(UBlackboardComponent* Blackboard, FBTFindNearestPlayerMemory* Memory, bool bValue) const;
	void UpdatePatrolLocation(UBlackboardComponent* Blackboard, const FVector& MyLocation, UNavigationSystemV1* NavSys) const;
};
