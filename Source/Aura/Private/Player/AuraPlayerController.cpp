// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/AuraPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Input/AuraInputComponent.h"
#include "AuraGameplayTags.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "UI/Widget/DamageTextWidgetComponent.h"
#include "GameFramework/Character.h"
#include "Character/AuraEnermy.h"


class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

AAuraPlayerController::AAuraPlayerController()
{
	//当前对象被复制时，是否应该复制它的属性。对于玩家控制器来说，通常需要设置为true，以便在网络游戏中正确同步玩家状态和行为。
	//主要作用就是允许玩家控制器在服务器和客户端之间进行通信和同步，使得玩家的输入、状态和行为能够在网络游戏中正确地反映出来。
	bReplicates = true;
}

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	// 鼠标拾取与敌人高亮仍需每帧执行（火球瞄准、UI 反馈都依赖它）
	CursorTrace();
}


void AAuraPlayerController::BroadcastDamageText_Implementation(const float DamageValue, ACharacter* HitCharacter, bool bIsCriticalHit, bool bIsBlockedHit)
{
	if (DamageTextWidgetComponent && IsValid(HitCharacter))
	{
		UDamageTextWidgetComponent* DamageTextCom = NewObject<UDamageTextWidgetComponent>(HitCharacter, DamageTextWidgetComponent);
		DamageTextCom->RegisterComponent();
		DamageTextCom->AttachToComponent(HitCharacter->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		DamageTextCom->SetDamageText(DamageValue, bIsCriticalHit, bIsBlockedHit);
	}
}



//输入系统就绪（拿 Subsystem + 加 MappingContext）
//鼠标显示设置（显示光标 + 样式）
//输入模式设置（Game + UI + 不锁鼠标 + 不隐藏光标）
//玩家控制器如果想要能够处理玩家输入，得先获得本地玩家子系统的引用，并将输入映射上下文添加到该子系统中。输入映射上下文定义了玩家输入与游戏行为之间的映射关系，确保玩家的输入能够正确地被识别和处理。
void AAuraPlayerController::BeginPlay()
{


	//调用父类的BeginPlay函数，确保父类的初始化逻辑能够正确执行。这是一个常见的做法，可以确保在子类中添加的逻辑不会干扰父类的正常功能。
	Super::BeginPlay();

	if (!AuraContext)
	{
		UE_LOG(LogTemp, Error, TEXT("AuraPlayerController [%s]: AuraContext is not set. Assign Input Mapping Context in BP_AuraPlayerController Class Defaults."), *GetName());
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());

	if (Subsystem)
	{
		//添加输入映射上下文到输入子系统中，参数0表示优先级，数值越小优先级越高。这样可以确保玩家控制器的输入映射在游戏中正确地被识别和处理。
		Subsystem->AddMappingContext(AuraContext, 0);
	}

	bShowMouseCursor = true;
	//设置默认鼠标光标
	DefaultMouseCursor = EMouseCursor::Default;
	//设置输入模式为游戏和UI模式，这样玩家既可以与游戏世界进行交互，也可以与用户界面进行交互。
	FInputModeGameAndUI InputModeData;
	//设置鼠标锁定行为为不锁定，这样玩家的鼠标光标可以自由移动，不会被限制在游戏视口内。
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	//设置在捕获鼠标时隐藏光标，这样当玩家按下鼠标按钮时，光标会暂时隐藏，以提供更好的游戏体验。
	InputModeData.SetHideCursorDuringCapture(false);
	//应用输入模式设置
	SetInputMode(InputModeData);


}

void AAuraPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (!MoveAction)
	{
		UE_LOG(LogTemp, Error, TEXT("AuraPlayerController [%s]: MoveAction is not set. Assign in BP_AuraPlayerController Class Defaults."), *GetName());
		return;
	}
	if (!InputConfig)
	{
		UE_LOG(LogTemp, Error, TEXT("AuraPlayerController [%s]: InputConfig is not set. Assign in BP_AuraPlayerController Class Defaults."), *GetName());
		return;
	}
	if (!LookAction)
	{
		UE_LOG(LogTemp, Error, TEXT("AuraPlayerController [%s]: LookAction is not set. Assign in BP_AuraPlayerController Class Defaults. 用于右键拖动旋转相机."), *GetName());
	}

	UAuraInputComponent* AuraInputComponent = CastChecked<UAuraInputComponent>(InputComponent);

	// WASD 移动
	AuraInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);

	// 右键拖动旋转相机：IA_Look 在 IMC 里绑定到 Mouse XY 2D 轴（不要绑成按钮）；
	// 是否真正驱动旋转由 bRightMouseDown 标志位在 Look() 里门控（标志位在 AbilityInputHeld/Released 里根据 InputTag.RMB 维护）
	AuraInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Look);

	// 能力输入：只绑定 Released 与 Held，Pressed 由 ASC 内部状态机处理（当前 ASC 没有 AbilityInputTagPressed）。
	// 这里把 Pressed 参数显式 cast 成对应的成员函数指针类型，让模板正确推导 PressedFuncType；
	// BindAbilityActions 内部有 if (PressedFunc) 判空，传入 nullptr 后不会真正绑定 Started 事件。
	AuraInputComponent->BindAbilityActions(
		InputConfig,
		this,
		static_cast<void(AAuraPlayerController::*)(FGameplayTag)>(nullptr),
		&ThisClass::AbilityInputReleased,
		&ThisClass::AbilityInputHeld);
}

UAuraAbilitySystemComponent* AAuraPlayerController::GetASC()
{
	if (AuraAbilitySystemComponent == nullptr)
	{
		AuraAbilitySystemComponent = Cast<UAuraAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn()));
	}
	return AuraAbilitySystemComponent;
}


//能力输入松开：统一转发到 ASC（LMB 已不再做寻路分支，火球走 GAS 自己的目标数据链路）
void AAuraPlayerController::AbilityInputReleased(FGameplayTag InputTag)
{
	// 右键松开时清除门控标志，Look() 不再驱动相机旋转
	if (InputTag.MatchesTagExact(FAuraGameplayTags::GetSingletonInstance().InputTag_RMB))
	{
		bRightMouseDown = false;
	}
	if (GetASC())
	{
		GetASC()->AbilityInputTagReleased(InputTag);
	}
}

//能力输入按住：统一转发到 ASC，激活 StarupInputTag 匹配的能力（LMB → GA_ProjectileSpell 火球）
void AAuraPlayerController::AbilityInputHeld(FGameplayTag InputTag)
{
	// 右键按住期间置位门控标志，Look() 据此决定是否驱动相机旋转
	if (InputTag.MatchesTagExact(FAuraGameplayTags::GetSingletonInstance().InputTag_RMB))
	{
		bRightMouseDown = true;
	}
	if (GetASC())
	{
		GetASC()->AbilityInputTagHeld(InputTag);
	}
}

void AAuraPlayerController::Move(const FInputActionValue& Value)
{
	//这是把值转换成一个二维向量，因为玩家的移动通常是基于水平和垂直轴的输入，例如WASD键或游戏手柄的左摇杆。通过将输入值转换为二维向量，可以更方便地处理玩家的移动逻辑，例如计算移动方向和速度。
	const FVector2D MovementVector = Value.Get<FVector2D>();
	//获取玩家控制器的控制旋转，这个旋转通常代表玩家当前的视角方向。通过获取控制旋转，可以将玩家的输入转换为相对于玩家视角的移动方向，使得玩家能够根据当前的视角进行移动。
	const FRotator Rotation = GetControlRotation();
	//创建一个新的旋转对象，表示玩家在水平面上的旋转。通过将控制旋转的俯仰和滚转分量设置为0，只保留偏航分量，可以确保玩家的移动方向只受到水平旋转的影响，而不会受到垂直旋转的干扰。
	const FRotator YawRotation(0, Rotation.Yaw, 0);
	//根据玩家的输入向量和水平旋转计算出前进方向的向量。通过将输入向量的X分量，可以得到玩家在当前视角下的前进方向。
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	//根据玩家的输入向量和水平旋转计算出右移方向的向量。通过将输入向量的Y分量，可以得到玩家在当前视角下的右移方向。
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	if(APawn* ControlledPawn = GetPawn<APawn>())
	{
		//根据玩家的输入向量和计算出的前进方向，添加一个沿着前进方向的移动输入。这样玩家就可以根据当前的视角进行前进或后退。
		ControlledPawn->AddMovementInput(ForwardDirection, MovementVector.Y);
		//根据玩家的输入向量和计算出的右移方向，添加一个沿着右移方向的移动输入。这样玩家就可以根据当前的视角进行左右移动。
		ControlledPawn->AddMovementInput(RightDirection, MovementVector.X);
	}

}

//右键按住时由 Enhanced Input 触发，把鼠标横向位移转成 ControlRotation 的 Yaw。
//AddYawInput 写入 APlayerController::RotationInput，引擎在 UpdateRotation 时累加到 ControlRotation.Yaw；
//SpringArm 开启 bUsePawnControlRotation 后会自动跟随，WASD 的 Move() 读 Yaw 也会同步方向。
//门控：只有 bRightMouseDown 为 true（右键按住）时才驱动旋转，避免鼠标自由移动时相机乱转。
void AAuraPlayerController::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxis = Value.Get<FVector2D>();

	// 右键未按住时直接返回，鼠标移动只用于光标定位，不旋转相机
	if (!bRightMouseDown)
	{
		return;
	}
	// 只用 X 轴做 Yaw（按方案只做左右旋转，Pitch 保持固定）
	AddYawInput(LookAxis.X * LookSensitivity);
}

void AAuraPlayerController::CursorTrace()
{
	//获取鼠标下的结果，使用ECC_Visibility通道进行碰撞检测，
	// 并将结果存储在CursorHit变量中。这样可以检测鼠标指针下是否有可见的对象，并获取相关信息，例如碰撞位置、碰撞对象等。

	GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);
	if (!CursorHit.bBlockingHit) return;

	//如果鼠标下有一个可见的对象，并且该对象实现了IEnermyInterface接口，那么将FocusedActor设置为该对象。这样可以让玩家控制器知道当前鼠标指针下的对象是什么，以便在游戏中进行相应的交互或显示相关信息。
	LastActor = FocusedActor;
	FocusedActor = Cast<IEnermyInterface>(CursorHit.GetActor());


	if (LastActor != FocusedActor)
	{
		if (LastActor) LastActor->UnHighlightEnermy();
		if (FocusedActor) FocusedActor->HighlightEnermy();
	}


}
