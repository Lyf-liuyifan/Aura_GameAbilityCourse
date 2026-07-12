#pragma once

#include "CoreMinimal.h"
#include "Game/AuraGameModeBase.h"
#include "Lab/Lockstep/LockstepTypes.h"
#include "LockstepGameMode.generated.h"

class ALockstepPawn;
class ALockstepPlayerController;

/**
 * 帧同步演示 GameMode：固定逻辑帧率收集输入、广播执行、支持本地回放。
 */
UCLASS()
class AURA_API ALockstepGameMode : public AAuraGameModeBase
{
	GENERATED_BODY()

public:
	ALockstepGameMode();

	virtual void Tick(float DeltaSeconds) override;
	virtual void StartPlay() override;

	/** Server 接收玩家本帧输入 */
	void ReceivePlayerInput(ALockstepPlayerController* PC, const FLockstepInput& Input);

	/** 本地玩家按 R 触发回放（由 GameState 在各端执行） */

	/** 逻辑帧率（Hz） */
	UPROPERTY(EditDefaultsOnly, Category = "Lockstep")
	float LogicTickRate = 15.f;

	/** 每格世界单位（cm） */
	UPROPERTY(EditDefaultsOnly, Category = "Lockstep")
	float GridSize = 100.f;

protected:
	void AdvanceLogicFrame();
	TArray<ALockstepPawn*> CollectLockstepPawns() const;
	void UpdatePawnRenderAlpha(float Alpha);

	int32 CurrentFrameNumber = 0;
	float LogicAccumulator = 0.f;

	/** 本逻辑帧各玩家已提交输入 */
	TMap<TWeakObjectPtr<ALockstepPlayerController>, FLockstepInput> PendingInputs;

	/** 录制用于回放的帧历史（仅 Server 侧备份，客户端以 GameState 为准） */
	UPROPERTY()
	TArray<FLockstepFrame> FrameHistory;
};
