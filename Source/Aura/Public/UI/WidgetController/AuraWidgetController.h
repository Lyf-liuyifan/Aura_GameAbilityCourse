// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AuraWidgetController.generated.h"

/**
 * 用户界面控制器基类，主要是为了广播事件给用户界面组件，改变用户界面组件的状态
 * 得获取用户状态组件，组件里有能力组件，能力组件里有属性组件，属性组件里有属性值，界面控制器获取属性值后广播事件给用户界面组件，改变用户界面组件的状态
 * 一是广播给UI组件，二是需要从用户状态组件获取属性值
 */



class UAbilitySystemComponent;
class UAttributeSet;
class PlayerController;

USTRUCT(BlueprintType)
struct FWidgetControllerAttributeParams
{

	GENERATED_BODY()
	FWidgetControllerAttributeParams() = default;
	FWidgetControllerAttributeParams(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS) :
		PlayerController(PC), PlayerState(PS), AbilitySystemComponent(ASC), AttributeSet(AS) {
	}

	TObjectPtr<APlayerController> PlayerController = nullptr;
	TObjectPtr<APlayerState> PlayerState = nullptr;
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = nullptr;
	TObjectPtr<UAttributeSet> AttributeSet = nullptr;


};


//向UI组件进行广播事件，改变UI组件的状态

UCLASS()
class AURA_API UAuraWidgetController : public UObject
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void SetWidgetController(const FWidgetControllerAttributeParams& WCParams);
	UFUNCTION(BlueprintCallable)
	virtual void BroadcastInitialValues();
	virtual void BindCallbacksToDependencies() {};

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
