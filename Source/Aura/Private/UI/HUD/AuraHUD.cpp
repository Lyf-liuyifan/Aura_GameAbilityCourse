// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/AuraHUD.h"
#include "UI/WidgetController/AuraOverlayWidgetController.h"
#include "UI/Widget/AuraUserWidget.h"
#include "Blueprint/UserWidget.h"

UAuraOverlayWidgetController* AAuraHUD::GetOverlayWidgetController(const FWidgetControllerAttributeParams& WCParams)
{
	if (OverlayWidgetController == nullptr)
	{
		OverlayWidgetController = NewObject<UAuraOverlayWidgetController>(this, OverlayWidgetControllerClass);
		OverlayWidgetController->SetWidgetController(WCParams);
		return OverlayWidgetController;
	}	
	return OverlayWidgetController;
}

//根据玩家控制器，玩家状态，能力系统组件，属性集来初始化界面组件
void AAuraHUD::InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{

	checkf(OverlayWidgetControllerClass, TEXT("OverlayWidgetControllerClass is null! Please assign it in the editor."));

	checkf(OverlayWidgetClass, TEXT("OverlayWidgetClass is null! Please assign it in the editor."));

	
		UUserWidget* TempWidget = CreateWidget<UUserWidget>(GetWorld(), OverlayWidgetClass);
		OverlayWidget = Cast<UAuraUserWidget>(TempWidget);
		
		FWidgetControllerAttributeParams WidgetControllerParams(PC, PS, ASC, AS);
		UAuraOverlayWidgetController* WidgetController = GetOverlayWidgetController(WidgetControllerParams);
		OverlayWidget->SetWidgetController(WidgetController);

		WidgetController->BroadcastInitialValues();

		if (OverlayWidget)
		{
			OverlayWidget->AddToViewport();
		}
}


