// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Player/AuraPlayerState.h"
#include "UI/HUD/AuraHUD.h"
#include "Kismet/GameplayStatics.h"
#include "Game/AuraGameModeBase.h"
#include "UI/WidgetController/AuraOverlayWidgetController.h"

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

void UAuraAbilitySystemLibrary::AddCharacterAbilities(UObject* WorldContextObject, UAbilitySystemComponent* ASC)
{
	AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (AuraGameMode == nullptr) return;

	UCharacterClassInfo* CharacterClassInfo = AuraGameMode->CharacterClassInfo;
	if (CharacterClassInfo == nullptr) return;

	auto CharacterAbilities = CharacterClassInfo->CommonAbilities;
	// Add character abilities
	for (const TSubclassOf<UGameplayAbility>& Ability : CharacterAbilities)
	{
		if (Ability)
		{
			// Assuming you have a function to add abilities to the character
			// AddAbilityToCharacter(Ability);
			FGameplayAbilitySpec AbilitySpec(Ability, 1);
			ASC->GiveAbility(AbilitySpec);
		}
	}
}
