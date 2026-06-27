#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "AuraLabDeveloperSettings.generated.h"

class AActor;
class UAbilitySystemComponent;
class UGameplayAbility;
class UGameplayEffect;
class AAuraLabTargetDummy;

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Aura Lab"))
class AURA_API UAuraLabDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UAuraLabDeveloperSettings();

	UPROPERTY(Config, EditAnywhere, Category = "Lab")
	bool bLabModeEnabled = true;

	UPROPERTY(Config, EditAnywhere, Category = "Lab|Classes")
	TSoftClassPtr<AAuraLabTargetDummy> DefaultTargetDummyClass;

	UPROPERTY(Config, EditAnywhere, Category = "Lab|Classes")
	TArray<TSubclassOf<UGameplayAbility>> DefaultLabAbilities;

	UPROPERTY(Config, EditAnywhere, Category = "Lab|Effects")
	TSoftClassPtr<UGameplayEffect> DefaultInstantEffectClass;

	UPROPERTY(Config, EditAnywhere, Category = "Lab|Effects")
	TSoftClassPtr<UGameplayEffect> DefaultDamageEffectClass;

	static const UAuraLabDeveloperSettings* Get();

	virtual FName GetCategoryName() const override { return FName(TEXT("Game")); }
};
