#include "Lab/Lockstep/LockstepPlayerController.h"

#include "Lab/Lockstep/LockstepGameMode.h"
#include "Lab/Lockstep/LockstepGameState.h"
#include "Lab/AuraLabLog.h"
#include "GameFramework/InputSettings.h"
#include "InputCoreTypes.h"

ALockstepPlayerController::ALockstepPlayerController()
{
	bReplicates = true;
}

void ALockstepPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	check(InputComponent);

	// 直接使用按键绑定，不依赖 DefaultInput.ini 的轴映射
	InputComponent->BindKey(EKeys::W, IE_Pressed, this, &ALockstepPlayerController::MoveForward);
	InputComponent->BindKey(EKeys::S, IE_Pressed, this, &ALockstepPlayerController::MoveBackward);
	InputComponent->BindKey(EKeys::D, IE_Pressed, this, &ALockstepPlayerController::MoveRight);
	InputComponent->BindKey(EKeys::A, IE_Pressed, this, &ALockstepPlayerController::MoveLeft);
	InputComponent->BindKey(EKeys::R, IE_Pressed, this, &ALockstepPlayerController::RequestReplay);
}

void ALockstepPlayerController::MoveForward()
{
	if (!IsLocalController()) return;
	CachedInput.MoveY = 1;
	ServerSubmitInput(CachedInput.MoveX, CachedInput.MoveY);
}

void ALockstepPlayerController::MoveBackward()
{
	if (!IsLocalController()) return;
	CachedInput.MoveY = -1;
	ServerSubmitInput(CachedInput.MoveX, CachedInput.MoveY);
}

void ALockstepPlayerController::MoveRight()
{
	if (!IsLocalController()) return;
	CachedInput.MoveX = 1;
	ServerSubmitInput(CachedInput.MoveX, CachedInput.MoveY);
}

void ALockstepPlayerController::MoveLeft()
{
	if (!IsLocalController()) return;
	CachedInput.MoveX = -1;
	ServerSubmitInput(CachedInput.MoveX, CachedInput.MoveY);
}

FLockstepInput ALockstepPlayerController::ConsumeCachedInput()
{
	FLockstepInput Result = CachedInput;
	CachedInput = FLockstepInput();
	return Result;
}

void ALockstepPlayerController::RequestReplay()
{
	if (!IsLocalController()) return;

	if (ALockstepGameState* GS = GetWorld() ? GetWorld()->GetGameState<ALockstepGameState>() : nullptr)
	{
		GS->StartLocalReplay();
	}
}

void ALockstepPlayerController::ServerSubmitInput_Implementation(int8 MoveX, int8 MoveY)
{
	if (ALockstepGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<ALockstepGameMode>() : nullptr)
	{
		FLockstepInput Input;
		Input.MoveX = MoveX;
		Input.MoveY = MoveY;
		GM->ReceivePlayerInput(this, Input);
	}
}
