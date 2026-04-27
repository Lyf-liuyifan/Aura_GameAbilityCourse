// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/AuraPlayerController.h"
#include "EnhancedInputSubsystems.h"

AAuraPlayerController::AAuraPlayerController()
{
	//当前对象被复制时，是否应该复制它的属性。对于玩家控制器来说，通常需要设置为true，以便在网络游戏中正确同步玩家状态和行为。
	//主要作用就是允许玩家控制器在服务器和客户端之间进行通信和同步，使得玩家的输入、状态和行为能够在网络游戏中正确地反映出来。
	bReplicates = true;

}


//输入系统就绪（拿 Subsystem + 加 MappingContext）
//鼠标显示设置（显示光标 + 样式）
//输入模式设置（Game + UI + 不锁鼠标 + 不隐藏光标）
void AAuraPlayerController::BeginPlay()
{


	//调用父类的BeginPlay函数，确保父类的初始化逻辑能够正确执行。这是一个常见的做法，可以确保在子类中添加的逻辑不会干扰父类的正常功能。
	Super::BeginPlay();

	check(AuraContext);

	//获取当前玩家控制器所属的本地玩家子系统，并将输入映射上下文添加到该子系统中。输入映射上下文定义了玩家输入与游戏行为之间的映射关系，确保玩家的输入能够正确地被识别和处理。
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());

	check(Subsystem);

	//添加输入映射上下文到输入子系统中，参数0表示优先级，数值越小优先级越高。这样可以确保玩家控制器的输入映射在游戏中正确地被识别和处理。
	Subsystem->AddMappingContext(AuraContext, 0);
	bShowMouseCursor = true;
	//设置默认鼠标光标
	DefaultMouseCursor = EMouseCursor::Default;
	//设置输入模式为游戏和UI模式，这样玩家既可以与游戏世界进行交互，也可以与用户界面进行交互。
	FInputModeGameAndUI InputModeData;
	//设置鼠标锁定行为为不锁定，这样玩家的鼠标光标可以自由移动，不会被限制在游戏视口内。
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	//设置在捕获鼠标时隐藏光标，这样当玩家按下鼠标按钮时，光标会暂时隐藏，以提供更好的游戏体验。
	InputModeData.SetHideCursorDuringCapture(true);
	//应用输入模式设置
	SetInputMode(InputModeData);


}
