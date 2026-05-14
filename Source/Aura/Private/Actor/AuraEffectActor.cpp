// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/AuraEffectActor.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Components/SphereComponent.h"
#include "Player/AuraPlayerState.h"
#include "AbilitySystem/AuraAttributeSet.h"

// Sets default values
AAuraEffectActor::AAuraEffectActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent")));
}


// Called when the game starts or when spawned
void AAuraEffectActor::BeginPlay()
{
	Super::BeginPlay();
	

}

void AAuraEffectActor::ApplyEffectToTarget(AActor* Target, TSubclassOf<UGameplayEffect> GameEffectClass, float Level)
{	
	//获得目标的ASC组件
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
	if (!TargetASC)
	{
		return;
	}
	check(GameEffectClass);
	//如果ASC组件存在，应用GameplayEffect
	FGameplayEffectContextHandle EffectContextHandle = TargetASC->MakeEffectContext();
	EffectContextHandle.AddSourceObject(this);
	FGameplayEffectSpecHandle EffectSpec = TargetASC->MakeOutgoingSpec(GameEffectClass, Level, EffectContextHandle);
	FActiveGameplayEffectHandle ActiveEffectHandle = TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpec.Data.Get());

	const bool bIsInfinite = EffectSpec.Data.Get()->Def.Get()->DurationPolicy == EGameplayEffectDurationType::Infinite;
	if (bIsInfinite && InfiniteEffectRemovalPolicy == EEffectRemovalPolicy::RemoveOnEndOverlap)
	{
		// 离开重叠时需要按 Handle 移除；必须在应用时登记，否则 OnEndOverlap 里 RemoveEffectFromTarget 找不到任何效果
		ActiveEffectHandles.Add(ActiveEffectHandle, TargetASC);
	}

}

void AAuraEffectActor::RemoveEffectFromTarget(AActor* Target)
{
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);

	if (!TargetASC)
	{
		return;
	}

	TArray<FActiveGameplayEffectHandle> HandlesToRemove;
	for(auto& HandleASCPair : ActiveEffectHandles)
	{
		FActiveGameplayEffectHandle ActiveEffectHandle = HandleASCPair.Key;
		if(HandleASCPair.Value == TargetASC)
		{
			TargetASC->RemoveActiveGameplayEffect(ActiveEffectHandle, 1);
			HandlesToRemove.Add(ActiveEffectHandle);
		}
	}
	for(int nums = 0; nums < HandlesToRemove.Num(); nums++)
	{
		ActiveEffectHandles.Remove(HandlesToRemove[nums]);
	}
}

void AAuraEffectActor::OnOverlap(AActor* TargetActor)
{
	//检查各个策略的应用时机，如果是ApplyOnOverlap则应用效果
	if (InstantEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(TargetActor, InstanceGamePlayEffectClass, EffectLevel);
	}
	if(DurationEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(TargetActor, DurationGamePlayEffectClass, EffectLevel);
	}
	if (InfiniteEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(TargetActor, InfiniteGamePlayEffectClass, EffectLevel);
	}
}

void AAuraEffectActor::OnEndOverlap(AActor* TargetActor)
{
	//检查各个策略的应用时机，如果是ApplyOnEndOverlap则应用效果
	if (InstantEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(TargetActor, InstanceGamePlayEffectClass, EffectLevel);
	}
	if (DurationEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(TargetActor, DurationGamePlayEffectClass, EffectLevel);
	}
	if (InfiniteEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(TargetActor, InfiniteGamePlayEffectClass, EffectLevel);
	}
	if(InfiniteEffectRemovalPolicy == EEffectRemovalPolicy::RemoveOnEndOverlap)
	{
		RemoveEffectFromTarget(TargetActor);
		UE_LOG(LogTemp, Log, TEXT("Remove Effect From Target Actor"));
	}
}


