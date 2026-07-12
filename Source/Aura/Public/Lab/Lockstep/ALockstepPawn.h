#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Lab/Lockstep/LockstepTypes.h"
#include "ALockstepPawn.generated.h"

class UStaticMeshComponent;

/**
 * 帧同步演示 Pawn：整数格子逻辑坐标 + 渲染插值，不接入 GAS。
 */
UCLASS()
class AURA_API ALockstepPawn : public APawn
{
	GENERATED_BODY()

public:
	ALockstepPawn();

	virtual void Tick(float DeltaSeconds) override;

	/** 逻辑帧执行移动，确定性整数步进 */
	void ApplyInput(const FLockstepInput& Input);

	/** 逻辑帧边界调用：保存上一格坐标用于插值 */
	void BeginLogicFrame();

	FIntPoint GetGridPosition() const { return GridPosition; }

	/** 重置到原点，供回放使用 */
	void ResetToOrigin();

	/** 由 GameMode 写入，用于世界坐标换算 */
	void SetGridSize(float InGridSize) { GridSize = InGridSize; }

	/** 渲染插值系数 0~1 */
	void SetRenderAlpha(float Alpha) { RenderAlpha = FMath::Clamp(Alpha, 0.f, 1.f); }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Lockstep")
	TObjectPtr<UStaticMeshComponent> DisplayMesh;

	/** 逻辑格子坐标 */
	FIntPoint GridPosition = FIntPoint::ZeroValue;

	/** 上一逻辑帧格子坐标，用于插值 */
	FIntPoint PrevGridPosition = FIntPoint::ZeroValue;

	float GridSize = 100.f;
	float RenderAlpha = 1.f;

	FVector GridToWorld(const FIntPoint& Grid) const;
};
