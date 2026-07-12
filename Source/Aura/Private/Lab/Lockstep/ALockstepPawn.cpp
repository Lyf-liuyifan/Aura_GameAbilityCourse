#include "Lab/Lockstep/ALockstepPawn.h"

#include "Components/StaticMeshComponent.h"
#include "Lab/AuraLabLog.h"
#include "UObject/ConstructorHelpers.h"

ALockstepPawn::ALockstepPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	DisplayMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DisplayMesh"));
	RootComponent = DisplayMesh;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		DisplayMesh->SetStaticMesh(CubeMesh.Object);
		DisplayMesh->SetWorldScale3D(FVector(0.5f));
	}
}

void ALockstepPawn::BeginPlay()
{
	Super::BeginPlay();
	ResetToOrigin();
}

void ALockstepPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 在上一逻辑帧与当前逻辑帧之间插值显示位置
	const FVector PrevWorld = GridToWorld(PrevGridPosition);
	const FVector CurWorld = GridToWorld(GridPosition);
	const FVector DisplayPos = FMath::Lerp(PrevWorld, CurWorld, RenderAlpha);
	SetActorLocation(DisplayPos);
}

void ALockstepPawn::BeginLogicFrame()
{
	PrevGridPosition = GridPosition;
}

void ALockstepPawn::ApplyInput(const FLockstepInput& Input)
{
	// 整数格子移动，边界 ±10 格
	const FIntPoint Delta(Input.MoveX, Input.MoveY);
	GridPosition += Delta;
	GridPosition.X = FMath::Clamp(GridPosition.X, -10, 10);
	GridPosition.Y = FMath::Clamp(GridPosition.Y, -10, 10);

	AURA_LAB_LOG(Log, TEXT("LockstepPawn %s Grid=(%d,%d)"), *GetName(), GridPosition.X, GridPosition.Y);
}

void ALockstepPawn::ResetToOrigin()
{
	GridPosition = FIntPoint::ZeroValue;
	PrevGridPosition = FIntPoint::ZeroValue;
	SetActorLocation(GridToWorld(GridPosition));
}

FVector ALockstepPawn::GridToWorld(const FIntPoint& Grid) const
{
	return FVector(Grid.X * GridSize, Grid.Y * GridSize, 50.f);
}
