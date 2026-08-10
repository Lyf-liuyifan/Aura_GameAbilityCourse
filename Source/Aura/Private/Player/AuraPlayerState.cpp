// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/AuraPlayerState.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Character/AuraCharacterBase.h"
#include "Net/UnrealNetwork.h"
#include "AbilitySystem/AuraAttributeSet.h"


AAuraPlayerState::AAuraPlayerState()
{

	// PlayerState 默认同步频率较低；这里提高频率，保证玩家状态在网络中更新更及时。
	NetUpdateFrequency = 100.0f;

	AbilitySystemComponent = CreateDefaultSubobject<UAuraAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AttributeSet = CreateDefaultSubobject<UAuraAttributeSet>(TEXT("AttributeSet"));

	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

}

void AAuraPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AAuraPlayerState, Level);
}

UAbilitySystemComponent* AAuraPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent.Get();
}

void AAuraPlayerState::OnRep_Level(int32 OldLevel)
{
	OnLevelChanged.Broadcast(Level);
}

void AAuraPlayerState::SetToLevel(int32 InLevels)
{
	if (!HasAuthority()) return;
	const int32 NewLevel = FMath::Max(1, InLevels);
	if (Level == NewLevel) return;
	Level = NewLevel;
	OnLevelChanged.Broadcast(Level);
	if (AAuraCharacterBase* Character = Cast<AAuraCharacterBase>(GetPawn()))
	{
		Character->RefreshAttributesForLevelUp();
	}
}
