#include "Lab/AuraLabDeveloperSettings.h"

UAuraLabDeveloperSettings::UAuraLabDeveloperSettings()
{
	CategoryName = TEXT("Game");
}

const UAuraLabDeveloperSettings* UAuraLabDeveloperSettings::Get()
{
	return GetDefault<UAuraLabDeveloperSettings>();
}
