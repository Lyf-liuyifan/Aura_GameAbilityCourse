#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Lab/Lockstep/LockstepTypes.h"
#include "LockstepGameState.generated.h"

/**
 * 帧同步 GameState：Multicast 广播逻辑帧，各端录制历史并支持本地回放。
 */
UCLASS()
class AURA_API ALockstepGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ALockstepGameState();

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastExecuteFrame(int32 FrameNumber, const TArray<FLockstepInput>& Inputs);

	void ExecuteFrameLocally(int32 FrameNumber, const TArray<FLockstepInput>& Inputs);

	/** 按 R 键：用已录制的输入流本地重放 */
	void StartLocalReplay();

	bool IsLocalReplaying() const { return bLocalReplaying; }

protected:
	UPROPERTY()
	TArray<FLockstepFrame> RecordedFrames;

	bool bLocalReplaying = false;
	int32 ReplayFrameIndex = 0;
	float ReplayAccumulator = 0.f;

	float GetLogicDelta() const;
};
