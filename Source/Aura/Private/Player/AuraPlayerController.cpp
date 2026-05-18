// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/AuraPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Input/AuraInputComponent.h"
#include "AuraGameplayTags.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "Character/AuraEnermy.h"


class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

AAuraPlayerController::AAuraPlayerController()
{
	//当前对象被复制时，是否应该复制它的属性。对于玩家控制器来说，通常需要设置为true，以便在网络游戏中正确同步玩家状态和行为。
	//主要作用就是允许玩家控制器在服务器和客户端之间进行通信和同步，使得玩家的输入、状态和行为能够在网络游戏中正确地反映出来。
	bReplicates = true;

	Spline = CreateDefaultSubobject<USplineComponent>("Spline");

}

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	CursorTrace();

}


//输入系统就绪（拿 Subsystem + 加 MappingContext）
//鼠标显示设置（显示光标 + 样式）
//输入模式设置（Game + UI + 不锁鼠标 + 不隐藏光标）
//玩家控制器如果想要能够处理玩家输入，得先获得本地玩家子系统的引用，并将输入映射上下文添加到该子系统中。输入映射上下文定义了玩家输入与游戏行为之间的映射关系，确保玩家的输入能够正确地被识别和处理。
void AAuraPlayerController::BeginPlay()
{


	//调用父类的BeginPlay函数，确保父类的初始化逻辑能够正确执行。这是一个常见的做法，可以确保在子类中添加的逻辑不会干扰父类的正常功能。
	Super::BeginPlay();

	check(AuraContext);

	//获取当前玩家控制器所属的本地玩家子系统，并将输入映射上下文添加到该子系统中。输入映射上下文定义了玩家输入与游戏行为之间的映射关系，确保玩家的输入能够正确地被识别和处理。
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
		//获取玩家输入组件，并将玩家输入事件与相应的函数绑定在一起，以便在游戏中处理玩家的输入行为。
		UAuraInputComponent* AuraInputComponent = CastChecked<UAuraInputComponent>(InputComponent);
		AuraInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
		AuraInputComponent->BindAbilityActions(InputConfig, this, &ThisClass::AbilityInputPressed, &ThisClass::AbilityInputReleased, &ThisClass::AbilityInputHeld);
}

UAuraAbilitySystemComponent* AAuraPlayerController::GetASC()
{
	if (AuraAbilitySystemComponent == nullptr)
	{
		AuraAbilitySystemComponent = Cast<UAuraAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn()));
	}
	return AuraAbilitySystemComponent;
}





//绑定到输入组件的函数
void AAuraPlayerController::AbilityInputPressed(FGameplayTag InputTag)
{
	//GEngine->AddOnScreenDebugMessage(1, 3.f, FColor::Red, *InputTag.ToString());
	if (InputTag.MatchesTagExact(FAuraGameplayTags::GetSingletonInstance().InputTag_LMB))
	{
		bIsTargeting = FocusedActor ? true : false;
		bAutoRunning = false;
	}
	
}

void AAuraPlayerController::AbilityInputReleased(FGameplayTag InputTag)
{
	

	if (!InputTag.MatchesTagExact(FAuraGameplayTags::GetSingletonInstance().InputTag_LMB))
	{
		bIsTargeting = FocusedActor ? true : false;
		bAutoRunning = false;
		if (GetASC())
		{
			GetASC()->AbilityInputTagReleased(InputTag);
		}
		
		return;
	}
	if (bIsTargeting)
	{//当鼠标在物体身上时
		if (GetASC())
		{
			GetASC()->AbilityInputTagReleased(InputTag);
		}
	}
	else
	{
		APawn* ControlledPawn = GetPawn();
		if (FollowTime <= ShortThreshold && ControlledPawn)
		{
			if (UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(this, ControlledPawn->GetActorLocation(), CachedDestination)) {
				Spline->ClearSplinePoints();
				for (const FVector& PointLoc : NavPath->PathPoints)
				{
					Spline->AddSplinePoint(PointLoc, ESplineCoordinateSpace::World);
					DrawDebugSphere(GetWorld(), PointLoc, 8.f, 8, FColor::Green, false, 5.f);
				}
				bAutoRunning = true;
			}
		}
		FollowTime = 0.f;
		bIsTargeting = false;
	}
}

void AAuraPlayerController::AbilityInputHeld(FGameplayTag InputTag)
{

	if (!InputTag.MatchesTagExact(FAuraGameplayTags::GetSingletonInstance().InputTag_LMB))
	{
		bIsTargeting = FocusedActor ? true : false;
		bAutoRunning = false;
		if (GetASC()) 
		{
			GetASC()->AbilityInputTagHeld(InputTag);
		}
		return;
	}
	
	if (bIsTargeting)
	{//当鼠标在物体身上时
		if (GetASC())
		{
			GetASC()->AbilityInputTagHeld(InputTag);
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Move"));
		FollowTime += GetWorld()->GetDeltaSeconds();

		FHitResult Hit;
		if (GetHitResultUnderCursor(ECC_Visibility, false, Hit))
		{
			CachedDestination = Hit.ImpactPoint;
		}
		if (APawn* ControlledPawn = GetPawn())
		{
			const FVector WorlddDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
			ControlledPawn->AddMovementInput(WorlddDirection);
		}
		
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

void AAuraPlayerController::CursorTrace()
{
	//获取鼠标下的结果，使用ECC_Visibility通道进行碰撞检测，
	// 并将结果存储在CursorHit变量中。这样可以检测鼠标指针下是否有可见的对象，并获取相关信息，例如碰撞位置、碰撞对象等。
	FHitResult CursorHit;
	GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);
	if (!CursorHit.bBlockingHit) return;

	//如果鼠标下有一个可见的对象，并且该对象实现了IEnermyInterface接口，那么将FocusedActor设置为该对象。这样可以让玩家控制器知道当前鼠标指针下的对象是什么，以便在游戏中进行相应的交互或显示相关信息。	
	LastActor = FocusedActor;
	FocusedActor = Cast<IEnermyInterface>(CursorHit.GetActor());

	/*
	* A.上一个物体空并且当前物体也是空
	*	- 什么都不做
	* B.上一个物体空并且当前物体不空
	*	- 可以触发一个事件，例如显示提示信息或高亮当前物体
	* C.上一个物体不空并且当前物体为空
	*	- 可以触发一个事件，例如隐藏提示信息或取消高亮
	* D.上一个物体不空并且当前物体不空但是是同一个物体
	* 
	* E.上一个物体不空并且当前物体不空但是是不同的物体
	*/

	if (LastActor == nullptr)
	{
		if (FocusedActor != nullptr)
		{
			//B.上一个物体空并且当前物体不空
			FocusedActor->HighlightEnermy();
		}else{
			//A.什么都不做
		}


	}
	else {//上一个物体不空
		if(FocusedActor != nullptr)
		{
			if(LastActor == FocusedActor)
			{
				//D.上一个物体不空并且当前物体不空但是是同一个物体
				return;
			}
			else {
				//E.上一个物体不空并且当前物体不空但是是不同的物体
				LastActor->UnHighlightEnermy();
				FocusedActor->HighlightEnermy();
			}
			
		}
		else {
			//C.上一个物体不空并且当前物体为空
			LastActor->UnHighlightEnermy();
		}
	}

}


