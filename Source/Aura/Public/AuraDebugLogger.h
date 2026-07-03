// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 调试日志辅助函数：把结构化日志写入 debug-6cd3b0.log（NDJSON）。
 * 仅用于本次调试，验证完成后移除。
 */
void AuraDebugLog(
    const FString& HypothesisId,
    const FString& RunId,
    const FString& Location,
    const FString& Message,
    const FString& DataJson = TEXT("{}"));
