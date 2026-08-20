// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Player/AuraPlayerState.h"
#include "UI/HUD/AuraHUD.h"
#include "Kismet/GameplayStatics.h"
#include "Game/AuraGameModeBase.h"
#include "AbilitySystem/AuraAbilityTypes.h"
#include "Character/AuraCharacterBase.h"
#include "UI/WidgetController/AuraOverlayWidgetController.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystem/Abilities/AuraRangedAttack.h"

UAuraOverlayWidgetController* UAuraAbilitySystemLibrary::GetOverlayWidgetController(const UObject* WorldContextObject)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{

		if (AAuraHUD* AuraHUD = Cast<AAuraHUD>(PC->GetHUD()))
		{
			AAuraPlayerState* PS = PC->GetPlayerState<AAuraPlayerState>();
			UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
			UAttributeSet* AS = PS->GetAttributeSet();
			const FWidgetControllerAttributeParams WidgetControllerParams(PC, PS, ASC, AS);
			return AuraHUD->GetOverlayWidgetController(WidgetControllerParams);

		}
	}
	return nullptr;
}

UMenuWidgetController* UAuraAbilitySystemLibrary::GetAttributeMenuWidgetController(const UObject* WorldContextObject)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{

		if (AAuraHUD* AuraHUD = Cast<AAuraHUD>(PC->GetHUD()))
		{
			AAuraPlayerState* PS = PC->GetPlayerState<AAuraPlayerState>();
			UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
			UAttributeSet* AS = PS->GetAttributeSet();
			const FWidgetControllerAttributeParams WidgetControllerParams(PC, PS, ASC, AS);
			return AuraHUD->GetMenuWidgetController(WidgetControllerParams);

		}
	}
	return nullptr;
}

void UAuraAbilitySystemLibrary::AddCharacterAbilities(UObject* WorldContextObject, UAbilitySystemComponent* ASC, ECharacterClass CharacterClass)
{
	AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (AuraGameMode == nullptr) return;

	if (CharacterClass == ECharacterClass::None) return;

	UCharacterClassInfo* CharacterClassInfo = AuraGameMode->CharacterClassInfo;
	if (CharacterClassInfo == nullptr) return;

	auto CharacterAbilities = CharacterClassInfo->CommonAbilities;
	// Add character abilities
	for (const TSubclassOf<UGameplayAbility>& Ability : CharacterAbilities)
	{
		if (Ability)
		{
			FGameplayAbilitySpec AbilitySpec(Ability, 1);
			ASC->GiveAbility(AbilitySpec);
		}
	}
	auto CharacterClassDefaultInfo = CharacterClassInfo->GetDefaultInfoForClass(CharacterClass);
	for (const TSubclassOf<UGameplayAbility>& Ability : CharacterClassDefaultInfo.StartupAbilities)
	{
		if (Ability)
		{
			// ASC::OnGiveAbility 只按当时 CDO 上的 AbilityTriggers 写入 TriggeredAbilityMap。
			// 蓝图序列化会把 PostInit 补上的 Event.Attack.Ranged 盖掉，必须在 Give 前再补一次；
			// Give 后再改 CDO，卸技能时会按新列表去删一条从未登记的 Tag。
			if (UAuraRangedAttack* RangedCDO = Ability->GetDefaultObject<UAuraRangedAttack>())
			{
				RangedCDO->EnsureAbilityTriggersRegistered();
			}

			FGameplayAbilitySpec AbilitySpec(Ability, 1);
			ASC->GiveAbility(AbilitySpec);
		}
	}
}

bool UAuraAbilitySystemLibrary::IsBlockedHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return AuraEffectContext->IsBlockedHit();
	}
	return false;
}

bool UAuraAbilitySystemLibrary::IsCriticalHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return AuraEffectContext->IsCriticalHit();
	}
	return false;
}

void UAuraAbilitySystemLibrary::GetLivePlayersWitinRadius(const UObject* WorldContextObject, const FVector& Origin, float Radius, TArray<AActor*>& OutActors,const TArray<AActor*>& ToIgnoreActors)
{
	FCollisionQueryParams SphereParams;
	SphereParams.AddIgnoredActors(ToIgnoreActors);
	TArray<FOverlapResult> OverlapResults;
	if(const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		World->OverlapMultiByObjectType(OverlapResults, Origin, FQuat::Identity, FCollisionObjectQueryParams(ECollisionChannel::ECC_Pawn), FCollisionShape::MakeSphere(Radius), SphereParams);
		for (const FOverlapResult& Result : OverlapResults)
		{
			if (Result.GetActor()->Implements<UCombatInterface>() && !ICombatInterface::Execute_IsDead(Result.GetActor()))
			{
				OutActors.AddUnique(ICombatInterface::Execute_GetAvatar(Result.GetActor()));
			}
		}
	}
}

void UAuraAbilitySystemLibrary::SetIsCraticalHit(FGameplayEffectContextHandle& EffectContextHandle, bool bInIsCriticalHit)
{
	if (FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		AuraEffectContext->SetIsCriticalHit(bInIsCriticalHit);
	}
}

void UAuraAbilitySystemLibrary::SetIsBlockedHit(FGameplayEffectContextHandle& EffectContextHandle, bool bInIsBlockedHit)
{
	if (FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		AuraEffectContext->SetIsBlockedHit(bInIsBlockedHit);
	}
}

