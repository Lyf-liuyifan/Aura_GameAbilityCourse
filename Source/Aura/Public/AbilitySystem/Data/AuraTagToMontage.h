// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AuraGameplayTags.h"
#include "Character/AuraCharacterBase.h"
#include "AuraTagToMontage.generated.h"

//最小的标签和动画蒙太奇的映射结构体
USTRUCT(BlueprintType)
struct FCharacterTagMontage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Montage")
	FGameplayTag MontageTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> Montage;

};


//每种角色对应的标签和动画蒙太奇的映射结构体
USTRUCT(BlueprintType)
struct FCharacterTagMontageContainer
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Montage")
	TArray<FCharacterTagMontage> CharacterTagMontages;
};


//角色种类标签和动画蒙太奇的映射结构体
USTRUCT(BlueprintType)
struct FCharacterTagMontageMap
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Montage")
	ECharacterCategory CharacterCategory;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Montage")
	FCharacterTagMontageContainer CharacterTagMontages;
};


/**
 * 
 */
UCLASS()
class AURA_API UAuraTagToMontage : public UDataAsset
{
	GENERATED_BODY()

public:


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage")
	TArray<FCharacterTagMontageMap> CharacterTagMontages;
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	UAnimMontage* GetMontageByTagFromData(AAuraCharacterBase* Character, FGameplayTag MontageTag);

};
