// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/AuraOverlayWidgetController.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Player/AuraPlayerState.h"


//初始化的时候需要广播属性值给UI组件，改变UI组件的状态
void UAuraOverlayWidgetController::BroadcastInitialValues()
{
	const UAuraAttributeSet* AuraAttributeSet = CastChecked<UAuraAttributeSet>(AttributeSet);

	OnHealthChanged.Broadcast(AuraAttributeSet->GetHealth());
	OnMaxHealthChanged.Broadcast(AuraAttributeSet->GetMaxHealth());
	OnManaChanged.Broadcast(AuraAttributeSet->GetMana());
	OnMaxManaChanged.Broadcast(AuraAttributeSet->GetMaxMana());
	OnXPChanged.Broadcast(AuraAttributeSet->GetXP());

	if (AAuraPlayerState* AuraPS = Cast<AAuraPlayerState>(PlayerState))
	{
		OnLevelChanged.Broadcast(AuraPS->GetPlayerLevel());
	}
}

void UAuraOverlayWidgetController::BindCallbacksToDependencies()
{
	//当属性值发生改变时，广播事件给UI组件，改变UI组件的状态
	//ASC侧值改变的时候通知控件控制器,随后控件控制器广播值变化
	const UAuraAttributeSet* AuraAttributeSet = CastChecked<UAuraAttributeSet>(AttributeSet);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UAuraAttributeSet::GetHealthAttribute()).AddUObject(this, &UAuraOverlayWidgetController::HealthChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UAuraAttributeSet::GetMaxHealthAttribute()).AddUObject(this, &UAuraOverlayWidgetController::MaxHealthChanged);	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UAuraAttributeSet::GetManaAttribute()).AddUObject(this, &UAuraOverlayWidgetController::ManaChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UAuraAttributeSet::GetMaxManaAttribute()).AddUObject(this, &UAuraOverlayWidgetController::MaxManaChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UAuraAttributeSet::GetXPAttribute()).AddUObject(this, &UAuraOverlayWidgetController::XPChanged);

	if (AAuraPlayerState* AuraPS = Cast<AAuraPlayerState>(PlayerState))
	{
		AuraPS->OnLevelChanged.AddDynamic(this, &UAuraOverlayWidgetController::HandlePlayerLevelChanged);
	}

	if (UAuraAbilitySystemComponent* AuraASC = Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent))
	{
		AuraASC->EffectAssetTags.AddLambda([this](const FGameplayTagContainer& TagContainer)
		{
			for (const FGameplayTag& Tag : TagContainer)
			{
				/**
				* Determine if this tag matches TagToCheck, expanding our parent tags
				* "A.1".MatchesTag("A") will return True, "A".MatchesTag("A.1") will return False
				* If TagToCheck is not Valid it will always return False
				*
				* @return True if this tag matches TagToCheck
				*/
				UE_LOG(LogTemp,Log,TEXT("GE Tag: %s"), *Tag.ToString());
				FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(FName("Message"));
 				if (Tag.MatchesTag(MessageTag)) {

					FUIWidgetRow* Row = this->GetDataTableRowByTag<FUIWidgetRow>(MessageDataTable, Tag);
					MessageWidgetRowDelegate.Broadcast(*Row);
				}
				
			}
		});
	}
}

void UAuraOverlayWidgetController::HealthChanged(const FOnAttributeChangeData& Data)
{
	OnHealthChanged.Broadcast(Data.NewValue);
}

void UAuraOverlayWidgetController::MaxHealthChanged(const FOnAttributeChangeData& Data)
{
	OnMaxHealthChanged.Broadcast(Data.NewValue);
}

void UAuraOverlayWidgetController::ManaChanged(const FOnAttributeChangeData& Data)
{
	OnManaChanged.Broadcast(Data.NewValue);
}

void UAuraOverlayWidgetController::MaxManaChanged(const FOnAttributeChangeData& Data)
{
	OnMaxManaChanged.Broadcast(Data.NewValue);
}

void UAuraOverlayWidgetController::XPChanged(const FOnAttributeChangeData& Data)
{
	OnXPChanged.Broadcast(Data.NewValue);
}

void UAuraOverlayWidgetController::HandlePlayerLevelChanged(int32 NewLevel)
{
	OnLevelChanged.Broadcast(NewLevel);
}
