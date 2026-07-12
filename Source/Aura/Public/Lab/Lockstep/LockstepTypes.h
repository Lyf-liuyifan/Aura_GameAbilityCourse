#pragma once

#include "CoreMinimal.h"
#include "LockstepTypes.generated.h"

/** 单玩家单逻辑帧的移动输入，使用整数避免浮点确定性问题 */
USTRUCT(BlueprintType)
struct FLockstepInput
{
	GENERATED_BODY()

	UPROPERTY()
	int8 MoveX = 0;

	UPROPERTY()
	int8 MoveY = 0;
};

/** 一帧的完整输入快照，用于回放 */
USTRUCT(BlueprintType)
struct FLockstepFrame
{
	GENERATED_BODY()

	UPROPERTY()
	int32 FrameNumber = 0;

	UPROPERTY()
	TArray<FLockstepInput> Inputs;
};
