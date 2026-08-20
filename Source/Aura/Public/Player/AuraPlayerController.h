// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "AuraPlayerController.generated.h"

/**
 * 玩家控制器：负责输入消费、鼠标拾取高亮、相机 Yaw 旋转（BG3 风格右键拖动）、GAS 输入转发。
 */
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class IEnermyInterface;
class UAuraInputConfig;
class UAuraAbilitySystemComponent;
class UDamageTextWidgetComponent;


UCLASS()
class AURA_API AAuraPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AAuraPlayerController();
	virtual void PlayerTick(float DeltaTime) override;

	/* Damage Text */
	UPROPERTY(EditDefaultsOnly, Category = "UI", BlueprintReadOnly)
	TSubclassOf<UDamageTextWidgetComponent> DamageTextWidgetComponent;

	UFUNCTION(Client, Reliable)
	void BroadcastDamageText(const float DamageValue, ACharacter* HitCharacter, bool bIsCriticalHit, bool bIsBlockedHit);

protected:
	virtual void BeginPlay() override;

	virtual void SetupInputComponent() override;


private:
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> AuraContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	/** 右键拖动旋转相机时使用的输入动作（IMC 中配置为 RightMouseButton 和弦） */
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	/** 鼠标横向滑动转 Yaw 的灵敏度系数 */
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float LookSensitivity = 1.5f;

	IEnermyInterface* LastActor;
	IEnermyInterface* FocusedActor;
	FHitResult CursorHit;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UAuraInputConfig> InputConfig;

	UPROPERTY()
	TObjectPtr<UAuraAbilitySystemComponent> AuraAbilitySystemComponent;

	UAuraAbilitySystemComponent* GetASC();


	/** 右键按住期间为 true，用于在 Look() 里门控鼠标移动是否驱动相机旋转 */
	bool bRightMouseDown = false;

	/** 能力输入松开：统一转发到 ASC，不再做 LMB 寻路分支 */
	void AbilityInputReleased(FGameplayTag InputTag);
	/** 能力输入按住：统一转发到 ASC，不再做 LMB 寻路分支 */
	void AbilityInputHeld(FGameplayTag InputTag);

	/** WASD 移动，方向相对 ControlRotation.Yaw（旋转相机后自动跟随） */
	void Move(const FInputActionValue& Value);

	/** 右键按住时由 Enhanced Input 触发，把鼠标横向位移转成 ControlRotation 的 Yaw */
	void Look(const FInputActionValue& Value);

	/** 每帧鼠标拾取，更新敌人高亮 */
	void CursorTrace();
};
