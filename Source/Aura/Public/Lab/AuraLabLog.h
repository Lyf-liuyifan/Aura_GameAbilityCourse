#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAuraLab, Log, All);

#define AURA_LAB_LOG(Verbosity, Format, ...) \
	UE_LOG(LogAuraLab, Verbosity, TEXT("[Lab] " Format), ##__VA_ARGS__)
