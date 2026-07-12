#include "Lab/LabNetSerializeTypes.h"

#include "Lab/AuraLabLog.h"

bool FLabNetSerializePayload::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	// 按字节读写 BuffId，float 使用引擎默认精度
	Ar << BuffId;
	Ar << Magnitude;
	bOutSuccess = true;
	return true;
}
