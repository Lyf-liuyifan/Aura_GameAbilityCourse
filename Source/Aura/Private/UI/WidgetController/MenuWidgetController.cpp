// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/MenuWidgetController.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AuraGameplayTags.h"





//确定属性值广播MenuData结构体
void UMenuWidgetController::BroadcastInitialValues()
{
	check(MenuData);
	UAuraAttributeSet* AS = CastChecked<UAuraAttributeSet>(AttributeSet);

	for (auto& Pair : AS->TagsToAttributes)
	{
		FAuraAttributeData Data = MenuData->FindAttributeDataByTag(Pair.Key);
		Data.AttributeValue = Pair.Value().GetNumericValue(AS);
		MenuDataDelegate.Broadcast(Data);
	}

}



//绑定ASC属性值变化委托，因为不同OverlayWidgetController
//MenuUI已经在蓝图处进行了初始化Controller的情况,使用了蓝图库函数
void UMenuWidgetController::BindCallbacksToDependencies()
{
	UAuraAttributeSet* AS = CastChecked<UAuraAttributeSet>(AttributeSet);

	for(auto& Pair : AS->TagsToAttributes)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Pair.Value()).AddLambda(
			[this, Pair, AS](const FOnAttributeChangeData& Data)
			{
				FAuraAttributeData ATData = MenuData->FindAttributeDataByTag(Pair.Key);
				ATData.AttributeValue = Pair.Value().GetNumericValue(AS);
				MenuDataDelegate.Broadcast(ATData);
				UE_LOG(LogTemp, Log, TEXT("ValueChange"));
			}
		);
	}

}


