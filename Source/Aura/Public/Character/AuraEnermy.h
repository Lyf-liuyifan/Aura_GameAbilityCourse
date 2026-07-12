// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/AuraCharacterBase.h"
#include "Interaction/EnermyInterface.h"
#include "AuraEnermy.generated.h"


class UBlackboardComponent;
class AAuraAIController;
class UBehaviorTree;
/**
 * 
 */
UCLASS()
class AURA_API AAuraEnermy : public AAuraCharacterBase, public IEnermyInterface
{
	GENERATED_BODY()
public:

	virtual void PossessedBy(AController* NewController)override;

	/* Enermy Interface*/
	AAuraEnermy();
	virtual void HighlightEnermy() override;
	virtual void UnHighlightEnermy() override;
	/* end Enermy Interface*/

	/* ICombatInterface */
	virtual int32 GetPlayerLevel() override { return Level; }
	virtual void Die() override;

	

	/* End ICombatInterface */

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Interaction")
	bool bIsHighlighted;

	/* UI Interface*/

	virtual void BindAttributeChangeDelegate() override;

	virtual void OnHealthChanged(const FOnAttributeChangeData& Data) override;

	virtual void OnMaxHealthChanged(const FOnAttributeChangeData& Data) override;

	/* UI Interface End*/


	/* Animation Interface*/
	UFUNCTION(BlueprintCallable)
	void GetHitReact(const FGameplayTag CallbackTag, int32 NewCount);

	/**
	 * 清除可能残留的 Effects.HitReact（例如伤害 GE 错误 Grant 了该 Tag），并恢复移速。
	 * 若 HitReact 能力仍通过 ActivationOwnedTags 持有该 Tag，移速保持为 0。
	 */
	void RecoverFromHitReact();

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsReactingToHit;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	float WalkSpeed = 250.f;

	/** 受击后多久尝试清除残留 HitReact Tag（秒） */
	UPROPERTY(EditDefaultsOnly, Category = "Combat", meta = (ClampMin = "0.1"))
	float HitReactRecoverDelay = 1.0f;

	FTimerHandle HitReactRecoverTimer;



	/* Animation Interface End*/

	/* Animation Properties */
	
	/* Animation Properties End */
	
	/* AI */


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	UPROPERTY()
	TObjectPtr<AAuraAIController> AuraAIController;

	UPROPERTY(BlueprintReadOnly)
	bool bHasSeenPlayer;

protected:
	virtual void BeginPlay() override;
	virtual	void InitAbilityActorInfo() override;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attribute")
	int32 Level;

	

};
