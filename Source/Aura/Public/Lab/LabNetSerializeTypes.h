#pragma once

#include "CoreMinimal.h"
#include "LabNetSerializeTypes.generated.h"

/**
 * Lab 用网络序列化结构体，演示 NetSerialize 自定义带宽与精度。
 */
USTRUCT(BlueprintType)
struct FLabNetSerializePayload
{
	GENERATED_BODY()

	UPROPERTY()
	uint8 BuffId = 0;

	UPROPERTY()
	float Magnitude = 0.f;

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
};

template<>
struct TStructOpsTypeTraits<FLabNetSerializePayload> : public TStructOpsTypeTraitsBase2<FLabNetSerializePayload>
{
	enum { WithNetSerializer = true };
};
