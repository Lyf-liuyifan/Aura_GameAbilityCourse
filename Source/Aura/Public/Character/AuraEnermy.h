// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/AuraCharacterBase.h"
#include "Interaction/EnermyInterface.h"
#include "AuraEnermy.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API AAuraEnermy : public AAuraCharacterBase, public IEnermyInterface
{
	GENERATED_BODY()
public:

	/* Enermy Interface*/
	AAuraEnermy();
	virtual void HighlightEnermy() override;
	virtual void UnHighlightEnermy() override;
	/* end Enermy Interface*/

	/* ICombatInterface */
	virtual int32 GetPlayerLevel() override { return Level; }

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
	virtual void GetHitByFire() override;
	void GetHitReactByFireBolt(const FGameplayTag CallbackTag, int32 NewCount);

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsReactingToHit;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	float WalkSpeed = 250.f;
	/* Animation Interface End*/

	/* Animation Properties */
	
	/* Animation Properties End */
	
protected:
	virtual void BeginPlay() override;
	virtual	void InitAbilityActorInfo() override;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attribute")
	int32 Level;
};
