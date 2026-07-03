// Fill out your copyright notice in the Description page of Project Settings.


#include "AuraDebugLogger.h"
#include "Misc/Paths.h"
#include "HAL/PlatformTime.h"
#include <fstream>
#include <string>

void AuraDebugLog(
    const FString& HypothesisId,
    const FString& RunId,
    const FString& Location,
    const FString& Message,
    const FString& DataJson)
{
    // 调试日志写入本次会话指定的 NDJSON 文件
    const FString DebugLogFilePath = FPaths::ProjectDir() / TEXT("debug-6cd3b0.log");
    const FString SessionId = TEXT("6cd3b0");
    const int64 Timestamp = static_cast<int64>(FPlatformTime::Seconds() * 1000.0);
    const FString Payload = FString::Printf(
        TEXT("{\"sessionId\":\"%s\",\"runId\":\"%s\",\"hypothesisId\":\"%s\",\"location\":\"%s\",\"message\":\"%s\",\"data\":%s,\"timestamp\":%lld}"),
        *SessionId, *RunId, *HypothesisId, *Location, *Message, *DataJson, Timestamp);

    const std::string Utf8Payload(TCHAR_TO_UTF8(*Payload));
    const std::string Utf8Path(TCHAR_TO_UTF8(*DebugLogFilePath));
    std::ofstream LogFile(Utf8Path, std::ios::app);
    if (LogFile.is_open())
    {
        LogFile << Utf8Payload << "\n";
        LogFile.close();
    }
}
