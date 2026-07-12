#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Lab/Lockstep/LockstepTypes.h"
#include "LockstepPlayerController.generated.h"

class ALockstepGameMode;

/**
 * 帧同步玩家控制器：采集 WASD 输入，经 Server RPC 提交给 GameMode。
 */
UCLASS()
class AURA_API ALockstepPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ALockstepPlayerController();

	FLockstepInput GetCachedInput() const { return CachedInput; }
	FLockstepInput ConsumeCachedInput();
	void RequestReplay();

protected:
	virtual void SetupInputComponent() override;

private:
	FLockstepInput CachedInput;

	void MoveForward();
	void MoveBackward();
	void MoveRight();
	void MoveLeft();

	UFUNCTION(Server, Reliable)
	void ServerSubmitInput(int8 MoveX, int8 MoveY);
};
