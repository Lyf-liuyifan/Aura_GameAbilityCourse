// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AuraEnemySpawner.generated.h"

class AAuraEnermy;

/**
 * 关卡里摆放的刷怪点。
 * BeginPlay 时以自身为圆心，在 NavMesh 可达范围内随机位置、随机种类刷出敌人。
 * 一关可以摆多个；刷几个、刷哪些由实例上的属性决定，不写进 GameMode。
 */
UCLASS()
class AURA_API AAuraEnemySpawner : public AActor
{
	GENERATED_BODY()

public:
	AAuraEnemySpawner();

protected:
	virtual void BeginPlay() override;

	/**
	 * 本点可刷出的敌人蓝图（如 BP_Goblin_Spear）。
	 * 每刷一只从中随机抽一个；空数组则整波跳过。
	 */
	UPROPERTY(EditAnywhere, Category = "Spawn")
	TArray<TSubclassOf<AAuraEnermy>> EnemyClasses;

	/** 本波刷出数量。第一关默认 6。 */
	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (ClampMin = "0"))
	int32 SpawnCount = 6;

	/** 以自身为圆心的刷怪半径（厘米）。点必须落在 NavMesh 上。 */
	UPROPERTY(EditAnywhere, Category = "Spawn", meta = (ClampMin = "0"))
	float SpawnRadius = 1500.f;

	/** 按配置刷出本波敌人。找不到可达点或类为空时跳过该只，不中断整波。 */
	void SpawnEnemies();
};
