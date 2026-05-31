// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "RotateToFaceTarget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRotateToFaceTargetSignature, FRotator, NewRotation);

UCLASS()
class AURA_API URotateToFaceTarget : public UAbilityTask
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Aura|AbilityTasks", meta = (DisplayName = "RotateToFaceTarget", HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"))
	static URotateToFaceTarget* CreateRotateToFaceTarget(UGameplayAbility* OwningAbility, FVector TargetLocation, float RotationSpeed = 0.f);

	virtual void Activate() override;
	virtual void TickTask(float DeltaTime) override;
	// Clean up and restore original rotation settings when the task ends
	virtual void OnDestroy(bool bInOwnerFinished) override;

	UPROPERTY(BlueprintAssignable)
	FRotateToFaceTargetSignature GetFaceRotationDelegate;

private:
	void BroadcastRotationFinished(const FRotator& NewRotation);

	FVector TargetLocation = FVector::ZeroVector;
	float RotationSpeed = 0.f;
	// Whether to restore the original bOrientRotationToMovement value when the task ends
	bool bOriginalOrientRotationToMovement = true;
	TWeakObjectPtr<class ACharacter> CachedCharacter;
};
