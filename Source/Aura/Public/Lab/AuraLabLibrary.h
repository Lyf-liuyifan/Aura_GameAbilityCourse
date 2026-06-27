#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayAbilitySpec.h"
#include "AuraLabLibrary.generated.h"

struct FGameplayAbilityActivationInfo;
class AActor;
class APlayerController;
class UAbilitySystemComponent;
class UGameplayAbility;

UCLASS()
class AURA_API UAuraLabLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Aura|Lab")
	static void LogNetRole(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "Aura|Lab")
	static void LogASCSummary(UAbilitySystemComponent* ASC);

	static void LogActivationInfo(AActor* AvatarActor, const FGameplayAbilityActivationInfo& ActivationInfo, const FString& AbilityName);

	UFUNCTION(BlueprintCallable, Category = "Aura|Lab")
	static UAbilitySystemComponent* GetLabASCFromController(APlayerController* PC);

	UFUNCTION(BlueprintCallable, Category = "Aura|Lab", meta = (WorldContext = "WorldContextObject"))
	static void GrantLabAbilities(UObject* WorldContextObject, APlayerController* PC, const TArray<TSubclassOf<UGameplayAbility>>& Abilities);

	UFUNCTION(BlueprintCallable, Category = "Aura|Lab", meta = (WorldContext = "WorldContextObject"))
	static bool ActivateLabAbilityByClassName(UObject* WorldContextObject, APlayerController* PC, FName AbilityClassName);

	UFUNCTION(BlueprintCallable, Category = "Aura|Lab")
	static void SimulateWrongClientGiveAbility(APlayerController* PC, TSubclassOf<UGameplayAbility> AbilityClass);

	static void RegisterConsoleCommands();
};
