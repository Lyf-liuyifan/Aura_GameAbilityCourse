// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "AuraHUD.generated.h"

/**
 * 
 * 要拥有指向用户界面组件的指针
 * 负责创建用户界面组件
 * A.HUD是一个Actor，所以它可以放在关卡里，或者被玩家控制器拥有
 * B.拥有一个大控件
 * C.这个大控件里有很多小控件
 * 
 * 创建完成员属性后，思考谁去创建这些组件，在哪里创建，什么时候创建
 */

class UUserWidget;
struct FWidgetControllerAttributeParams;
class UAuraOverlayWidgetController;
class UAuraUserWidget;
class UAbilitySystemComponent;
class UAttributeSet;

UCLASS()
class AURA_API AAuraHUD : public AHUD
{
	GENERATED_BODY()
public:
	UPROPERTY()
	TObjectPtr<UAuraUserWidget> OverlayWidget;

	
	UAuraOverlayWidgetController* GetOverlayWidgetController(const FWidgetControllerAttributeParams& WCParams);

	//根据玩家控制器，玩家状态，能力系统组件，属性集来初始化界面组件
	void InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS);

protected:

private:
	//我们不知道要创建的是什么组件，所以用TSubclassOf来指定一个组件的类
	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> OverlayWidgetClass;

	UPROPERTY()
	TObjectPtr<UAuraOverlayWidgetController> OverlayWidgetController;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UAuraOverlayWidgetController> OverlayWidgetControllerClass;
};
