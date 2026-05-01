// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "AuraHUD.generated.h"

/**
 * 用来绘制用户界面
 * 所以要拥有指向用户界面组件的指针
 * A.HUD是一个Actor，所以它可以放在关卡里，或者被玩家控制器拥有
 * B.拥有一个大控件
 * C.这个大控件里有很多小控件
 * 
 * 创建完成员属性后，思考谁去创建这些组件，在哪里创建，什么时候创建
 */

class UUserWidget;

UCLASS()
class AURA_API AAuraHUD : public AHUD
{
	GENERATED_BODY()
public:
	UPROPERTY()
	TObjectPtr<UUserWidget> OverlayWidget;
protected:
	virtual void BeginPlay() override;

private:
	//我们不知道要创建的是什么组件，所以用TSubclassOf来指定一个组件的类
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> OverlayWidgetClass;
};
