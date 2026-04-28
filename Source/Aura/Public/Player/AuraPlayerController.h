// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AuraPlayerController.generated.h"

/**
 * 
 */
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class IEnermyInterface;

//我的想法就是首先玩家控制器类想要获取玩家输入并且转换成输出
//首先得先获取玩家输入inputcomponent，然后把玩家输入和函数绑定在一起，最后在游戏中处理玩家的输入行为

UCLASS()
class AURA_API AAuraPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AAuraPlayerController();
	virtual void PlayerTick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	//用来设置玩家控制器的输入组件，绑定玩家输入事件和函数，以便在游戏中处理玩家的输入行为。
	virtual void SetupInputComponent() override;


private:
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> AuraContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	void Move(const FInputActionValue& Value);

	void CursorTrace();

	IEnermyInterface* LastActor;
	IEnermyInterface* FocusedActor;
};
