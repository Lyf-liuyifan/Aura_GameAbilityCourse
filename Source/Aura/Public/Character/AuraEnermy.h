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
	AAuraEnermy();
	virtual void HighlightEnermy() override;
	virtual void UnHighlightEnermy() override;
};
