// Fill out your copyright notice in the Description page of Project Settings.


#include "AuraAssetManager.h"
#include "GameplayTagsManager.h"
#include "AbilitySystemGlobals.h"
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

	//服务器收到客户端发送的数据必须调用的初始化函数
	UAbilitySystemGlobals::Get().InitGlobalData();

}
