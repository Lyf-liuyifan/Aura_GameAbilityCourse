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
		//调用绑定函数，绑定属性改变的事件，当属性值发生改变时，广播事件给UI组件，改变UI组件的状态
		OverlayWidgetController->BindCallbacksToDependencies();
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


UMenuWidgetController* AAuraHUD::GetMenuWidgetController(const FWidgetControllerAttributeParams& WCParams)
{
	if (MenuWidgetController == nullptr)
	{
		MenuWidgetController = NewObject<UMenuWidgetController>(this, MenuWidgetControllerClass);
		MenuWidgetController->SetWidgetController(WCParams);
		MenuWidgetController->BindCallbacksToDependencies();
		return MenuWidgetController;
	}
	return MenuWidgetController;
}


void AAuraHUD::InitMenuWidget(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	checkf(MenuWidgetControllerClass, TEXT("MenuWidgetControllerClass is null! Please assign it in the editor."));
	checkf(MenuWidgetClass, TEXT("MenuWidgetClass is null! Please assign it in the editor."));

	UUserWidget* TempWidget = CreateWidget<UUserWidget>(GetWorld(), MenuWidgetClass);
	MenuWidget = Cast<UAuraUserWidget>(TempWidget);

	FWidgetControllerAttributeParams WidgetControllerParams(PC, PS, ASC, AS);
	UMenuWidgetController* WidgetController = GetMenuWidgetController(WidgetControllerParams);
	MenuWidget->SetWidgetController(WidgetController);

	WidgetController->BroadcastInitialValues();

	
}
