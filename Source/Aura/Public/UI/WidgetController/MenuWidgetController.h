// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "AbilitySystem/Data/AttributeDataAsset.h"
#include "MenuWidgetController.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAuraAttributeDataSignature, const FAuraAttributeData&, MenuData);


/**
 * 属性值变化，广播，控制器收到属性值变化的通知
 */
UCLASS(BlueprintType, Blueprintable)
class AURA_API UMenuWidgetController : public UAuraWidgetController
{
	GENERATED_BODY()
	
public:
	virtual void BroadcastInitialValues()override;
	virtual void BindCallbacksToDependencies()override;



	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FAuraAttributeDataSignature MenuDataDelegate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAttributeDataAsset> MenuData;
};
