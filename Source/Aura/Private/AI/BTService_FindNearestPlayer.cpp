// Fill out your copyright notice in the Description page of Project Settings.



#include "AI/BTService_FindNearestPlayer.h"

#include "AIController.h"

#include "BehaviorTree/BlackboardComponent.h"

#include "BehaviorTree/BehaviorTree.h"

#include "Character/AuraCharacter.h"

#include "DrawDebugHelpers.h"

#include "Engine/OverlapResult.h"

#include "Kismet/KismetSystemLibrary.h"



void UBTService_FindNearestPlayer::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)

{

	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIController = OwnerComp.GetAIOwner();

	if (!AIController) return;

	APawn* MyPawn = AIController->GetPawn();

	if (!MyPawn) return;

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();

	if (!Blackboard) return;

	const FVector MyLocation = MyPawn->GetActorLocation();



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



	AActor* NearestPlayer = nullptr;

	float ClosestDistance = MAX_FLT;

	const FVector Forward = MyPawn->GetActorForwardVector();

	const float CosHalfAngle = FMath::Cos(FMath::DegreesToRadians(HalfSightAngle));

	if (bHit)

	{

		for (AActor* Actor : OverlappedActors)

		{

			if (!IsValid(Actor)) continue;



			const FVector ToTarget = Actor->GetActorLocation() - MyLocation;

			const float Distance = ToTarget.Size();

			if (Distance > SearchRadius) continue;



			const FVector DirToTarget = ToTarget.GetSafeNormal();

			const float Dot = FVector::DotProduct(Forward, DirToTarget);

			if (Dot < CosHalfAngle) continue;



			if (bUseLineOfSight)

			{

				FHitResult Hit;

				FCollisionQueryParams Params(SCENE_QUERY_STAT(AISight), false, MyPawn);

				const bool bBlocked = MyPawn->GetWorld()->LineTraceSingleByChannel(

					Hit, MyLocation, Actor->GetActorLocation(), ECC_Visibility, Params);

				if (bBlocked && Hit.GetActor() != Actor) continue;

			}



			if (Distance < ClosestDistance)

			{

				ClosestDistance = Distance;

				NearestPlayer = Actor;

			}

		}

	}



	if (NearestPlayer)

	{

		Blackboard->SetValueAsObject(BlackboardKeyTargetActor.SelectedKeyName, NearestPlayer);

		if (!BlackboardKeyHasValidTarget.SelectedKeyName.IsNone())

		{

			Blackboard->SetValueAsBool(BlackboardKeyHasValidTarget.SelectedKeyName, true);

		}

	}

	else

	{

		Blackboard->SetValueAsObject(BlackboardKeyTargetActor.SelectedKeyName, nullptr);

		if (!BlackboardKeyHasValidTarget.SelectedKeyName.IsNone())

		{

			Blackboard->SetValueAsBool(BlackboardKeyHasValidTarget.SelectedKeyName, false);

		}

	}



#if ENABLE_DRAW_DEBUG

	DrawDebugSphere(MyPawn->GetWorld(), MyLocation, SearchRadius, 16,

		NearestPlayer ? FColor::Green : FColor::Red, false, Interval);

	const FVector ConeLeft = Forward.RotateAngleAxis(-HalfSightAngle, FVector::UpVector);

	const FVector ConeRight = Forward.RotateAngleAxis(HalfSightAngle, FVector::UpVector);

	DrawDebugLine(MyPawn->GetWorld(), MyLocation,

		MyLocation + ConeLeft * SearchRadius, FColor::Yellow, false, Interval);

	DrawDebugLine(MyPawn->GetWorld(), MyLocation,

		MyLocation + ConeRight * SearchRadius, FColor::Yellow, false, Interval);

#endif

}



UBTService_FindNearestPlayer::UBTService_FindNearestPlayer()

{

	NodeName = TEXT("Find Nearest Player");

	Interval = 0.5;

}



void UBTService_FindNearestPlayer::InitializeFromAsset(UBehaviorTree& Asset)

{

	Super::InitializeFromAsset(Asset);

	UBlackboardData* BBAsset = Asset.GetBlackboardAsset();

	if (ensure(BBAsset))

	{

		BlackboardKeyTargetActor.ResolveSelectedKey(*BBAsset);

		BlackboardKeyHasValidTarget.ResolveSelectedKey(*BBAsset);

	}

}

