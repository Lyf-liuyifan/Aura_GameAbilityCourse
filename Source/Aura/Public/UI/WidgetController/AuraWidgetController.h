// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AuraWidgetController.generated.h"

/**
 * 用户界面控制器基类，主要是为了广播事件给用户界面组件，改变用户界面组件的状态
 * 得获取用户状态组件，组件里有能力组件，能力组件里有属性组件，属性组件里有属性值，界面控制器获取属性值后广播事件给用户界面组件，改变用户界面组件的状态
 * 
 */

class UAbilitySystemComponent;
class UAttributeSet;
class PlayerController;

UCLASS()
class AURA_API UAuraWidgetController : public UObject
{
	GENERATED_BODY()
public:

protected:
	UPROPERTY()
	TObjectPtr<APlayerController> PlayerController;

	UPROPERTY()
	TObjectPtr<APlayerState> PlayerState;

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

};
