// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BTService_FindNearestPlayer.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Character/AuraCharacter.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "NavigationSystem.h"

UBTService_FindNearestPlayer::UBTService_FindNearestPlayer()
{
	NodeName = TEXT("Find Nearest Player");
	Interval = 0.5f;
}

void UBTService_FindNearestPlayer::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	UBlackboardData* BBAsset = Asset.GetBlackboardAsset();
	if (ensure(BBAsset))
	{
		BlackboardKeyTargetToFollow.ResolveSelectedKey(*BBAsset);
		BlackboardKeyHasValidTarget.ResolveSelectedKey(*BBAsset);
		BlackboardKeybHasSeenPlayer.ResolveSelectedKey(*BBAsset);
		BlackboardKeyPatrolLocation.ResolveSelectedKey(*BBAsset);
		BlackboardKeyLocationFarFromTarget.ResolveSelectedKey(*BBAsset);
		BlackboardKeyIsMelee.ResolveSelectedKey(*BBAsset);
		BlackboardKeybNeedsToRetreat.ResolveSelectedKey(*BBAsset);
		BlackboardKeybNeedsToAdvance.ResolveSelectedKey(*BBAsset);
		BlackboardKeyLastLocation.ResolveSelectedKey(*BBAsset);
		BlackboardKeyDistanceToTarget.ResolveSelectedKey(*BBAsset);
	}
}

AAuraCharacter* UBTService_FindNearestPlayer::FindPlayerInSight(APawn* MyPawn, const FVector& MyLocation, float& OutDistance) const
{
	OutDistance = MAX_FLT;
	AAuraCharacter* NearestPlayer = nullptr;

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(MyPawn);

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	TArray<AActor*> OverlappedActors;
	const bool bHit = UKismetSystemLibrary::SphereOverlapActors(
		MyPawn,
		MyLocation,
		SearchRadius,
		ObjectTypes,
		AAuraCharacter::StaticClass(),
		ActorsToIgnore,
		OverlappedActors
	);

	if (!bHit)
	{
		return nullptr;
	}

	const FVector Forward = MyPawn->GetActorForwardVector();
	const float CosHalfAngle = FMath::Cos(FMath::DegreesToRadians(HalfSightAngle));

	for (AActor* Actor : OverlappedActors)
	{
		if (!IsValid(Actor)) continue;

		const FVector ToTarget = Actor->GetActorLocation() - MyLocation;
		const float Distance = ToTarget.Size();

		const FVector DirToTarget = ToTarget.GetSafeNormal();
		if (FVector::DotProduct(Forward, DirToTarget) < CosHalfAngle) continue;

		if (bUseLineOfSight)
		{
			FHitResult Hit;
			FCollisionQueryParams Params(SCENE_QUERY_STAT(AISight), false, MyPawn);
			const bool bBlocked = MyPawn->GetWorld()->LineTraceSingleByChannel(
				Hit, MyLocation, Actor->GetActorLocation(), ECC_Visibility, Params);
			if (bBlocked && Hit.GetActor() != Actor) continue;
		}

		if (Distance < OutDistance)
		{
			OutDistance = Distance;
			NearestPlayer = Cast<AAuraCharacter>(Actor);
		}
	}

	return NearestPlayer;
}

AAuraCharacter* UBTService_FindNearestPlayer::FindPlayerOnNavMesh(
	UWorld* World,
	const FVector& MyLocation,
	float& OutDistance,
	bool& bOutOnNavMesh) const
{
	OutDistance = MAX_FLT;
	bOutOnNavMesh = false;

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetNavigationSystem(World);
	if (!NavSys)
	{
		return nullptr;
	}

	TArray<AActor*> AllPlayers;
	UGameplayStatics::GetAllActorsOfClass(World, AAuraCharacter::StaticClass(), AllPlayers);

	AAuraCharacter* NearestPlayer = nullptr;

	for (AActor* Actor : AllPlayers)
	{
		AAuraCharacter* Player = Cast<AAuraCharacter>(Actor);
		if (!IsValid(Player)) continue;

		const FVector PlayerLocation = Player->GetActorLocation();
		FNavLocation NavLocation;
		if (!NavSys->ProjectPointToNavigation(PlayerLocation, NavLocation, NavProjectionExtent))
		{
			continue;
		}

		bOutOnNavMesh = true;

		const float Distance = FVector::Dist(MyLocation, PlayerLocation);
		if (Distance < OutDistance)
		{
			OutDistance = Distance;
			NearestPlayer = Player;
		}
	}

	return NearestPlayer;
}

void UBTService_FindNearestPlayer::UpdateRangedDistanceData(
	UBlackboardComponent* Blackboard,
	const FVector& MyLocation,
	const FVector& PlayerLocation,
	UNavigationSystemV1* NavSys) const
{
	if (!Blackboard || !NavSys) return;

	const float Distance = FVector::Dist(MyLocation, PlayerLocation);
	const bool bTooClose = Distance < RetreatDistance;
	const bool bTooFar = Distance > RangedAttackMax;

	if (!BlackboardKeybNeedsToRetreat.SelectedKeyName.IsNone())
	{
		Blackboard->SetValueAsBool(BlackboardKeybNeedsToRetreat.SelectedKeyName, bTooClose);
	}
	if (!BlackboardKeybNeedsToAdvance.SelectedKeyName.IsNone())
	{
		Blackboard->SetValueAsBool(BlackboardKeybNeedsToAdvance.SelectedKeyName, bTooFar && !bTooClose);
	}

	if (bTooClose && !BlackboardKeyLocationFarFromTarget.SelectedKeyName.IsNone())
	{
		const FVector AwayFromPlayer = (MyLocation - PlayerLocation).GetSafeNormal();
		const FVector RetreatLocation = PlayerLocation + AwayFromPlayer * RangedAttackDistance;

		FNavLocation NavLocation;
		if (NavSys->ProjectPointToNavigation(RetreatLocation, NavLocation, NavProjectionExtent))
		{
			Blackboard->SetValueAsVector(BlackboardKeyLocationFarFromTarget.SelectedKeyName, NavLocation.Location);
		}
	}
	else if (bTooFar && !BlackboardKeyLocationFarFromTarget.SelectedKeyName.IsNone())
	{
		const FVector AwayFromPlayer = (MyLocation - PlayerLocation).GetSafeNormal();
		const FVector RetreatLocation = PlayerLocation + AwayFromPlayer * RangedAttackDistance;

		FNavLocation NavLocation;
		if (NavSys->ProjectPointToNavigation(RetreatLocation, NavLocation, NavProjectionExtent))
		{
			Blackboard->SetValueAsVector(BlackboardKeyLocationFarFromTarget.SelectedKeyName, NavLocation.Location);
		}
	}
	else if (!BlackboardKeyLocationFarFromTarget.SelectedKeyName.IsNone())
	{
		Blackboard->ClearValue(BlackboardKeyLocationFarFromTarget.SelectedKeyName);
	}
}

void UBTService_FindNearestPlayer::ClearCombatBlackboard(UBlackboardComponent* Blackboard, FBTFindNearestPlayerMemory* Memory) const
{
	if (!Blackboard) return;

	SetHasSeenPlayer(Blackboard, Memory, false);

	if (!BlackboardKeyTargetToFollow.SelectedKeyName.IsNone())
	{
		Blackboard->ClearValue(BlackboardKeyTargetToFollow.SelectedKeyName);
	}
	if (!BlackboardKeyHasValidTarget.SelectedKeyName.IsNone())
	{
		Blackboard->SetValueAsBool(BlackboardKeyHasValidTarget.SelectedKeyName, false);
	}
	if (!BlackboardKeyLocationFarFromTarget.SelectedKeyName.IsNone())
	{
		Blackboard->ClearValue(BlackboardKeyLocationFarFromTarget.SelectedKeyName);
	}
	if (!BlackboardKeybNeedsToRetreat.SelectedKeyName.IsNone())
	{
		Blackboard->SetValueAsBool(BlackboardKeybNeedsToRetreat.SelectedKeyName, false);
	}
	if (!BlackboardKeybNeedsToAdvance.SelectedKeyName.IsNone())
	{
		Blackboard->SetValueAsBool(BlackboardKeybNeedsToAdvance.SelectedKeyName, false);
	}
	if (!BlackboardKeyDistanceToTarget.SelectedKeyName.IsNone())
	{
		Blackboard->ClearValue(BlackboardKeyDistanceToTarget.SelectedKeyName);
	}
}

void UBTService_FindNearestPlayer::SetHasSeenPlayer(
	UBlackboardComponent* Blackboard,
	FBTFindNearestPlayerMemory* Memory,
	bool bValue) const
{
	if (Memory)
	{
		Memory->bHasSeenPlayer = bValue;
	}
	if (Blackboard && !BlackboardKeybHasSeenPlayer.SelectedKeyName.IsNone())
	{
		Blackboard->SetValueAsBool(BlackboardKeybHasSeenPlayer.SelectedKeyName, bValue);
	}
}

void UBTService_FindNearestPlayer::UpdatePatrolLocation(
	UBlackboardComponent* Blackboard,
	const FVector& MyLocation,
	UNavigationSystemV1* NavSys) const
{
	if (!Blackboard || !NavSys || BlackboardKeyPatrolLocation.SelectedKeyName.IsNone()) return;

	FNavLocation NavLocation;
	if (NavSys->GetRandomReachablePointInRadius(MyLocation, SearchRadius, NavLocation))
	{
		Blackboard->SetValueAsVector(BlackboardKeyPatrolLocation.SelectedKeyName, NavLocation.Location);
	}
}

void UBTService_FindNearestPlayer::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	// --- 基础引用 ---
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return;

	APawn* MyPawn = AIController->GetPawn();
	if (!MyPawn) return;

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard) return;

	// 每个敌人独立的战斗态记忆（避免 BT Service 类成员被所有敌人共享）
	FBTFindNearestPlayerMemory* Memory = reinterpret_cast<FBTFindNearestPlayerMemory*>(NodeMemory);

	UWorld* World = MyPawn->GetWorld();
	if (!World) return;

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetNavigationSystem(World);

	const FVector MyLocation = MyPawn->GetActorLocation();
	const bool bIsMelee = MyPawn->ActorHasTag(FName("Melee"));

	// 写入 IsMelee，供 BT Decorator 分流近战 / 远程
	if (!BlackboardKeyIsMelee.SelectedKeyName.IsNone())
	{
		Blackboard->SetValueAsBool(BlackboardKeyIsMelee.SelectedKeyName, bIsMelee);
	}

	// Damage 感知会先写黑板 HasSeenPlayer；此处同步到节点 Memory，避免巡逻态把目标清掉
	if (Memory && !Memory->bHasSeenPlayer
		&& !BlackboardKeybHasSeenPlayer.SelectedKeyName.IsNone()
		&& Blackboard->GetValueAsBool(BlackboardKeybHasSeenPlayer.SelectedKeyName))
	{
		Memory->bHasSeenPlayer = true;
	}

	// bHasSeenPlayer：false=巡逻态，true=战斗态（首次发现玩家后永久切换，直到玩家离开 NavMesh）
	const bool bHasSeenPlayer = Memory ? Memory->bHasSeenPlayer : false;

	float DistanceToTarget = MAX_FLT;
	AAuraCharacter* TargetPlayer = nullptr;
	bool bPlayerOnNavMesh = false;

	if (!bHasSeenPlayer)
	{
		// 【巡逻态】情况1：用视野锥 + 距离 + 可选 LOS 做首次发现
		TargetPlayer = FindPlayerInSight(MyPawn, MyLocation, DistanceToTarget);

		if (TargetPlayer)
		{
			// 【情况2】首次发现玩家 → 进入战斗态，BT 走战斗 Sequence
			const FVector PlayerLocation = TargetPlayer->GetActorLocation();

			SetHasSeenPlayer(Blackboard, Memory, true);
			if (!BlackboardKeyTargetToFollow.SelectedKeyName.IsNone())
			{
				Blackboard->SetValueAsObject(BlackboardKeyTargetToFollow.SelectedKeyName, TargetPlayer);
			}
			if (!BlackboardKeyHasValidTarget.SelectedKeyName.IsNone())
			{
				Blackboard->SetValueAsBool(BlackboardKeyHasValidTarget.SelectedKeyName, true);
			}
			if (!BlackboardKeyLastLocation.SelectedKeyName.IsNone())
			{
				Blackboard->SetValueAsVector(BlackboardKeyLastLocation.SelectedKeyName, PlayerLocation);
			}
			if (!BlackboardKeyDistanceToTarget.SelectedKeyName.IsNone())
			{
				Blackboard->SetValueAsFloat(BlackboardKeyDistanceToTarget.SelectedKeyName, DistanceToTarget);
			}

			// 远程：计算三区距离标志（太近后撤 / 太远追击 / 理想区 EQS）
			if (!bIsMelee && NavSys)
			{
				UpdateRangedDistanceData(Blackboard, MyLocation, PlayerLocation, NavSys);
			}
			else
			{
				if (!BlackboardKeybNeedsToRetreat.SelectedKeyName.IsNone())
				{
					Blackboard->SetValueAsBool(BlackboardKeybNeedsToRetreat.SelectedKeyName, false);
				}
				if (!BlackboardKeybNeedsToAdvance.SelectedKeyName.IsNone())
				{
					Blackboard->SetValueAsBool(BlackboardKeybNeedsToAdvance.SelectedKeyName, false);
				}
				if (!BlackboardKeyLocationFarFromTarget.SelectedKeyName.IsNone())
				{
					Blackboard->ClearValue(BlackboardKeyLocationFarFromTarget.SelectedKeyName);
				}
			}
		}
		else
		{
			// 【情况1】未发现玩家 → HasValidTarget=false，BT 走巡逻 Sequence
			ClearCombatBlackboard(Blackboard, Memory);
			if (NavSys)
			{
				UpdatePatrolLocation(Blackboard, MyLocation, NavSys);
			}
		}
	}
	else
	{
		// 【战斗态】情况3：全图追踪 NavMesh 上的玩家，不再受视野锥限制
		TargetPlayer = FindPlayerOnNavMesh(World, MyLocation, DistanceToTarget, bPlayerOnNavMesh);

		if (TargetPlayer && bPlayerOnNavMesh)
		{
			// 玩家仍在 NavMesh 上 → 持续追击，BT 保持战斗 Sequence
			const FVector PlayerLocation = TargetPlayer->GetActorLocation();

			if (!BlackboardKeyTargetToFollow.SelectedKeyName.IsNone())
			{
				Blackboard->SetValueAsObject(BlackboardKeyTargetToFollow.SelectedKeyName, TargetPlayer);
			}
			if (!BlackboardKeyHasValidTarget.SelectedKeyName.IsNone())
			{
				Blackboard->SetValueAsBool(BlackboardKeyHasValidTarget.SelectedKeyName, true);
			}
			if (!BlackboardKeyLastLocation.SelectedKeyName.IsNone())
			{
				Blackboard->SetValueAsVector(BlackboardKeyLastLocation.SelectedKeyName, PlayerLocation);
			}
			if (!BlackboardKeyDistanceToTarget.SelectedKeyName.IsNone())
			{
				Blackboard->SetValueAsFloat(BlackboardKeyDistanceToTarget.SelectedKeyName, DistanceToTarget);
			}

			// 情况4（<RetreatDistance 后撤）/ 情况5（>RangedAttackMax 追击）/ 情况2（理想区 EQS）
			if (!bIsMelee && NavSys)
			{
				UpdateRangedDistanceData(Blackboard, MyLocation, PlayerLocation, NavSys);
			}
			else
			{
				if (!BlackboardKeybNeedsToRetreat.SelectedKeyName.IsNone())
				{
					Blackboard->SetValueAsBool(BlackboardKeybNeedsToRetreat.SelectedKeyName, false);
				}
				if (!BlackboardKeybNeedsToAdvance.SelectedKeyName.IsNone())
				{
					Blackboard->SetValueAsBool(BlackboardKeybNeedsToAdvance.SelectedKeyName, false);
				}
				if (!BlackboardKeyLocationFarFromTarget.SelectedKeyName.IsNone())
				{
					Blackboard->ClearValue(BlackboardKeyLocationFarFromTarget.SelectedKeyName);
				}
			}
		}
		else
		{
			// 【情况3 边界】玩家离开 NavMesh → 退出战斗态，回巡逻
			ClearCombatBlackboard(Blackboard, Memory);
			if (NavSys)
			{
				UpdatePatrolLocation(Blackboard, MyLocation, NavSys);
			}
		}
	}

#if ENABLE_DRAW_DEBUG
	// 巡逻态画视野锥，战斗态只画搜索球
	const FVector Forward = MyPawn->GetActorForwardVector();
	const bool bInCombat = Memory && Memory->bHasSeenPlayer;

	DrawDebugSphere(World, MyLocation, SearchRadius, 16,
		TargetPlayer ? FColor::Green : FColor::Red, false, Interval);

	if (!bInCombat)
	{
		const FVector ConeLeft = Forward.RotateAngleAxis(-HalfSightAngle, FVector::UpVector);
		const FVector ConeRight = Forward.RotateAngleAxis(HalfSightAngle, FVector::UpVector);

		DrawDebugLine(World, MyLocation, MyLocation + ConeLeft * SearchRadius, FColor::Yellow, false, Interval);
		DrawDebugLine(World, MyLocation, MyLocation + ConeRight * SearchRadius, FColor::Yellow, false, Interval);
	}
#endif
}
