// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/AbilityTasks/RotateToFaceTarget.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

URotateToFaceTarget* URotateToFaceTarget::CreateRotateToFaceTarget(UGameplayAbility* OwningAbility, FVector InTargetLocation, float InRotationSpeed)
{
	URotateToFaceTarget* Task = NewAbilityTask<URotateToFaceTarget>(OwningAbility);
	Task->TargetLocation = InTargetLocation;
	Task->RotationSpeed = InRotationSpeed;
	return Task;
}

void URotateToFaceTarget::Activate()
{
	Super::Activate();

	if (AActor* Avatar = Ability->GetCurrentActorInfo()->AvatarActor.Get())
	{
		CachedCharacter = Cast<ACharacter>(Avatar);
		if (CachedCharacter.IsValid())
		{
			UCharacterMovementComponent* MovementComp = CachedCharacter->GetCharacterMovement();
			bOriginalOrientRotationToMovement = MovementComp->bOrientRotationToMovement;
			MovementComp->bOrientRotationToMovement = false;

			if (RotationSpeed <= 0.f)
			{
				RotationSpeed = MovementComp->RotationRate.Yaw;
			}
		}
	}

	bTickingTask = true;
}

void URotateToFaceTarget::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);

	if (!CachedCharacter.IsValid())
	{
		EndTask();
		return;
	}

	const FVector ToTarget = TargetLocation - CachedCharacter->GetActorLocation();
	if (ToTarget.IsNearlyZero())
	{
		BroadcastRotationFinished(CachedCharacter->GetActorRotation());
		EndTask();
		return;
	}

	FRotator TargetRotation = ToTarget.Rotation();
	TargetRotation.Pitch = 0.f;
	TargetRotation.Roll = 0.f;

	const FRotator CurrentRotation = CachedCharacter->GetActorRotation();
	const FRotator NewRotation = FMath::RInterpConstantTo(CurrentRotation, TargetRotation, DeltaTime, RotationSpeed);
	CachedCharacter->SetActorRotation(NewRotation);

	if (FMath::Abs(FMath::FindDeltaAngleDegrees(NewRotation.Yaw, TargetRotation.Yaw)) <= 1.f)
	{
		CachedCharacter->SetActorRotation(TargetRotation);
		BroadcastRotationFinished(TargetRotation);
		EndTask();
	}
}

void URotateToFaceTarget::OnDestroy(bool bInOwnerFinished)
{
	if (CachedCharacter.IsValid())
	{
		CachedCharacter->GetCharacterMovement()->bOrientRotationToMovement = bOriginalOrientRotationToMovement;
	}

	Super::OnDestroy(bInOwnerFinished);
}

void URotateToFaceTarget::BroadcastRotationFinished(const FRotator& NewRotation)
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		GetFaceRotationDelegate.Broadcast(NewRotation);
	}
}
