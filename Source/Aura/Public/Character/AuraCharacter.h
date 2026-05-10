// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/AuraCharacterBase.h"
#include "AbilitySystemComponent.h"
#include "AuraCharacter.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API AAuraCharacter : public AAuraCharacterBase
{
	GENERATED_BODY()
public:
	AAuraCharacter();

	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	/* ICombatInterface */
	int32 GetPlayerLevel();
	/* End ICombatInterface */

private:
	/*AAuraCharacter API*/
	virtual void InitAbilityActorInfo() override;
};
