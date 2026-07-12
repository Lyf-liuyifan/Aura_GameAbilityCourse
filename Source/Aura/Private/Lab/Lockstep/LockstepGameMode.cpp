#include "Lab/Lockstep/LockstepGameMode.h"

#include "Lab/Lockstep/ALockstepPawn.h"
#include "Lab/Lockstep/LockstepGameState.h"
#include "Lab/Lockstep/LockstepPlayerController.h"
#include "Lab/AuraLabLog.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"

ALockstepGameMode::ALockstepGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	DefaultPawnClass = ALockstepPawn::StaticClass();
	PlayerControllerClass = ALockstepPlayerController::StaticClass();
	GameStateClass = ALockstepGameState::StaticClass();
}

void ALockstepGameMode::StartPlay()
{
	Super::StartPlay();

	// 初始化所有 LockstepPawn 的格子尺寸
	for (ALockstepPawn* Pawn : CollectLockstepPawns())
	{
		if (Pawn)
		{
			Pawn->SetGridSize(GridSize);
		}
	}

	AURA_LAB_LOG(Warning, TEXT("LockstepGameMode 启动 | LogicTickRate=%.1f GridSize=%.0f"), LogicTickRate, GridSize);
}

void ALockstepGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const float LogicDelta = LogicTickRate > 0.f ? 1.f / LogicTickRate : 0.1f;

	// 本地回放由 GameState 在各端执行
	if (ALockstepGameState* GS = GetWorld()->GetGameState<ALockstepGameState>())
	{
		if (GS->IsLocalReplaying())
		{
			UpdatePawnRenderAlpha(LogicAccumulator / LogicDelta);
			return;
		}
	}

	LogicAccumulator += DeltaSeconds;

	// 固定步长推进逻辑帧（仅 Server 驱动）
	while (LogicAccumulator >= LogicDelta)
	{
		LogicAccumulator -= LogicDelta;

		if (HasAuthority())
		{
			AdvanceLogicFrame();
		}
	}

	UpdatePawnRenderAlpha(LogicDelta > 0.f ? LogicAccumulator / LogicDelta : 1.f);
}

void ALockstepGameMode::UpdatePawnRenderAlpha(float Alpha)
{
	for (ALockstepPawn* Pawn : CollectLockstepPawns())
	{
		if (Pawn)
		{
			Pawn->SetRenderAlpha(Alpha);
		}
	}
}

void ALockstepGameMode::ReceivePlayerInput(ALockstepPlayerController* PC, const FLockstepInput& Input)
{
	if (!HasAuthority() || !PC) return;
	PendingInputs.Add(PC, Input);
}

void ALockstepGameMode::AdvanceLogicFrame()
{
	++CurrentFrameNumber;

	// 按 PlayerId 排序收集各玩家输入
	TArray<APlayerController*> PCs;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PC = It->Get())
		{
			PCs.Add(PC);
		}
	}
	PCs.Sort([](const APlayerController& A, const APlayerController& B)
	{
		const APlayerState* PSA = A.GetPlayerState<APlayerState>();
		const APlayerState* PSB = B.GetPlayerState<APlayerState>();
		if (PSA && PSB) return PSA->GetPlayerId() < PSB->GetPlayerId();
		return &A < &B;
	});

	TArray<FLockstepInput> FrameInputs;
	for (APlayerController* PC : PCs)
	{
		FLockstepInput Input;
		if (ALockstepPlayerController* LockstepPC = Cast<ALockstepPlayerController>(PC))
		{
			if (const FLockstepInput* Found = PendingInputs.Find(LockstepPC))
			{
				Input = *Found;
			}
		}
		FrameInputs.Add(Input);
	}

	PendingInputs.Empty();

	// 录制帧历史供 R 键回放
	FLockstepFrame Record;
	Record.FrameNumber = CurrentFrameNumber;
	Record.Inputs = FrameInputs;
	FrameHistory.Add(Record);

	AURA_LAB_LOG(Warning, TEXT("Lockstep 广播帧 %d | 输入数=%d"), CurrentFrameNumber, FrameInputs.Num());

	// 通过 GameState Multicast 到所有客户端（GameMode 不在客户端存在）
	if (ALockstepGameState* GS = GetWorld()->GetGameState<ALockstepGameState>())
	{
		GS->MulticastExecuteFrame(CurrentFrameNumber, FrameInputs);
	}
}

TArray<ALockstepPawn*> ALockstepGameMode::CollectLockstepPawns() const
{
	TArray<ALockstepPawn*> Result;
	if (!GetWorld()) return Result;

	for (TActorIterator<ALockstepPawn> It(GetWorld()); It; ++It)
	{
		Result.Add(*It);
	}
	Result.Sort([](const ALockstepPawn& A, const ALockstepPawn& B)
	{
		return A.GetName() < B.GetName();
	});
	return Result;
}

