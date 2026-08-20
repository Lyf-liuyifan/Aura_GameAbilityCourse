// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/AuraEnemySpawner.h"

AAuraEnemySpawner::AAuraEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AAuraEnemySpawner::BeginPlay()
{
	Super::BeginPlay();
}

void AAuraEnemySpawner::SpawnEnemies()
{
}
