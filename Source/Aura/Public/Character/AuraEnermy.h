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

protected:
	virtual void BeginPlay() override;
	virtual	void InitAbilityActorInfo() override;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attribute")
	int32 Level;
};
