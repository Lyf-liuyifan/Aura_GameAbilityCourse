// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AuraUserWidget.generated.h"

/**
 * 用户界面组件基类
 * 用户界面要接受来自界面控制器的广播来更改用户界面，所以是监听者、订阅者
 * 订阅者要拥有发布者的指针
 */


UCLASS()
class AURA_API UAuraUserWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	/*
	* AuraUserWidget Interface
	*/

	//要设置拥有的控制器
	//以便在蓝图中设置控制器
	UFUNCTION()
	void SetWidgetController(UObject* InWidgetController);

	/*
	*  End AuraUserWidget Interface
	*/

	//以便蓝图可以访问到用户界面控制器
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UObject> WidgetController;

protected:
	//职责是：通知蓝图“控制器已经注入完成，现在可以开始绑定和刷新UI了”
	//所以SetWidgetController是专门触发这个事件的，事件的名字叫WidgetControllerSet
	//蓝图实现这个事件，在事件里绑定界面控制器的广播事件，刷新UI
	//SetWidgetController是告诉自己控制器已经设置好了
	UFUNCTION(BlueprintImplementableEvent)
	void WidgetControllerSet();
};
