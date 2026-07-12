#include "Lab/Lockstep/LockstepGameState.h"

#include "Lab/Lockstep/ALockstepPawn.h"
#include "Lab/Lockstep/LockstepGameMode.h"
#include "Lab/AuraLabLog.h"
#include "EngineUtils.h"

ALockstepGameState::ALockstepGameState()
{
	PrimaryActorTick.bCanEverTick = true;
}

float ALockstepGameState::GetLogicDelta() const
{
	if (const ALockstepGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<ALockstepGameMode>() : nullptr)
	{
		return GM->LogicTickRate > 0.f ? 1.f / GM->LogicTickRate : 0.1f;
	}
	return 1.f / 15.f;
}

void ALockstepGameState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bLocalReplaying) return;

	ReplayAccumulator += DeltaSeconds;
	const float LogicDelta = GetLogicDelta();

	while (ReplayAccumulator >= LogicDelta)
	{
		ReplayAccumulator -= LogicDelta;
		if (ReplayFrameIndex >= RecordedFrames.Num())
		{
			bLocalReplaying = false;
			AURA_LAB_LOG(Warning, TEXT("Lockstep 回放结束"));
			break;
		}

		const FLockstepFrame& Frame = RecordedFrames[ReplayFrameIndex];
		ExecuteFrameLocally(Frame.FrameNumber, Frame.Inputs);
		++ReplayFrameIndex;
	}
}

void ALockstepGameState::MulticastExecuteFrame_Implementation(int32 FrameNumber, const TArray<FLockstepInput>& Inputs)
{
	ExecuteFrameLocally(FrameNumber, Inputs);

	// 各端录制输入流，供本地回放
	FLockstepFrame Record;
	Record.FrameNumber = FrameNumber;
	Record.Inputs = Inputs;
	RecordedFrames.Add(Record);
}

void ALockstepGameState::ExecuteFrameLocally(int32 FrameNumber, const TArray<FLockstepInput>& Inputs)
{
	UWorld* World = GetWorld();
	if (!World) return;

	TArray<ALockstepPawn*> Pawns;
	for (TActorIterator<ALockstepPawn> It(World); It; ++It)
	{
		Pawns.Add(*It);
	}
	Pawns.Sort([](const ALockstepPawn& A, const ALockstepPawn& B)
	{
		return A.GetName() < B.GetName();
	});

	for (int32 i = 0; i < Pawns.Num(); ++i)
	{
		ALockstepPawn* Pawn = Pawns[i];
		if (!Pawn) continue;

		Pawn->BeginLogicFrame();

		const FLockstepInput Input = Inputs.IsValidIndex(i) ? Inputs[i] : FLockstepInput();
		if (Input.MoveX != 0 || Input.MoveY != 0)
		{
			Pawn->ApplyInput(Input);
			AURA_LAB_LOG(Log, TEXT("Lockstep Frame=%d Pawn=%s Input=(%d,%d) Grid=(%d,%d)"),
				FrameNumber, *Pawn->GetName(), Input.MoveX, Input.MoveY,
				Pawn->GetGridPosition().X, Pawn->GetGridPosition().Y);
		}
	}
}

void ALockstepGameState::StartLocalReplay()
{
	if (RecordedFrames.Num() == 0)
	{
		AURA_LAB_LOG(Warning, TEXT("Lockstep 回放：无录制帧，请先操作一段时间"));
		return;
	}

	bLocalReplaying = true;
	ReplayFrameIndex = 0;
	ReplayAccumulator = 0.f;

	UWorld* World = GetWorld();
	if (!World) return;

	for (TActorIterator<ALockstepPawn> It(World); It; ++It)
	{
		It->ResetToOrigin();
	}

	AURA_LAB_LOG(Warning, TEXT("Lockstep 开始回放 | 共 %d 帧"), RecordedFrames.Num());
}
