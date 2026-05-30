// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AbilityTasks/TargetDataUnderMouse.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"

UTargetDataUnderMouse* UTargetDataUnderMouse::CreateTargetDataUnderMouse(UGameplayAbility* OwningAbility)
{
	UTargetDataUnderMouse* Task = NewAbilityTask<UTargetDataUnderMouse>(OwningAbility);
	return Task;
}

void UTargetDataUnderMouse::Activate()
{
	//已经有AbilitySystemComponent
	if (AbilitySystemComponent.IsValid())
	{
		//AbilitySystemComponent.AbilityActorInfo->PlayerController()
		const bool bIsLocallyControlled = Ability->GetCurrentActorInfo()->IsLocallyControlled();
		if (bIsLocallyControlled)
		{//TODO: we are client,so send target data to server
			SendMouseDataToServer();
		}
		else 
		{//TODO: we are Server,so listen for target data
			FGameplayAbilitySpecHandle SpecHandle;
			FPredictionKey ActivationPredictionKey = GetActivationPredictionKey();
			AbilitySystemComponent.Get()->AbilityTargetDataSetDelegate(GetAbilitySpecHandle(), GetActivationPredictionKey()).AddUObject(this, &UTargetDataUnderMouse::OnTargetDataReplicatedCallback);
			const bool bCalledDelegate = AbilitySystemComponent.Get()->CallReplicatedTargetDataDelegatesIfSet(SpecHandle, ActivationPredictionKey);
			if (!bCalledDelegate)
			{
				SetWaitingOnRemotePlayerData();
			}
		
		}

		
	}
}

void UTargetDataUnderMouse::SendMouseDataToServer()
{
	FScopedPredictionWindow ScopePrediction(AbilitySystemComponent.Get());
	
	//预测的内容
	APlayerController * PC = Ability->GetCurrentActorInfo()->PlayerController.Get();
	FHitResult UnderMouseHitRes;
	PC->GetHitResultUnderCursor(ECC_Visibility, false, UnderMouseHitRes);

	FGameplayAbilityTargetData_SingleTargetHit* HitData = new FGameplayAbilityTargetData_SingleTargetHit();
	HitData->HitResult = UnderMouseHitRes;
	
	FGameplayAbilityTargetDataHandle DataHandle;
	DataHandle.Add(HitData);
	//发去服务器
	AbilitySystemComponent->ServerSetReplicatedTargetData(GetAbilitySpecHandle(),
		GetActivationPredictionKey(),
		DataHandle,
		FGameplayTag(),
		AbilitySystemComponent->ScopedPredictionKey);

	if (ShouldBroadcastAbilityTaskDelegates())
	{//如果能还在激活状态，广播数据
		MouseTargetDataDelegate.Broadcast(DataHandle);
	}

	UE_LOG(LogTemp, Log, TEXT("Client Get Mouse Target Task Activate"));
}

void UTargetDataUnderMouse::OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& DataHandle, FGameplayTag ActivationTag)
{
	AbilitySystemComponent->ConsumeAllReplicatedData(GetAbilitySpecHandle(), GetActivationPredictionKey());
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		MouseTargetDataDelegate.Broadcast(DataHandle);
	}
}


