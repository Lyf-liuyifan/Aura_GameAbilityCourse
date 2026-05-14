// Fill out your copyright notice in the Description page of Project Settings.


#include "AuraAssetManager.h"
#include "GameplayTagsManager.h"
#include "AuraGameplayTags.h"


UAuraAssetManager& UAuraAssetManager::Get()
{
	check(GEngine);
	UAuraAssetManager* AssetManager = Cast<UAuraAssetManager>(GEngine->AssetManager);
	return *AssetManager;
}

void UAuraAssetManager::StartInitialLoading()
{
	//
	Super::StartInitialLoading();
	FAuraGameplayTags::InitializeNativeGameplayTags();
}
