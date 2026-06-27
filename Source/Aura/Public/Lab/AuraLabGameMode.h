#pragma once

#include "CoreMinimal.h"
#include "Game/AuraGameModeBase.h"
#include "AuraLabGameMode.generated.h"

class AAuraLabTargetDummy;
class UGameplayAbility;

UCLASS()
class AURA_API AAuraLabGameMode : public AAuraGameModeBase
{
	GENERATED_BODY()

public:
	AAuraLabGameMode();

	virtual void StartPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Lab")
	TSubclassOf<AAuraLabTargetDummy> TargetDummyClass;

	UPROPERTY(EditDefaultsOnly, Category = "Lab")
	int32 DummyCount = 2;

	UPROPERTY(EditDefaultsOnly, Category = "Lab")
	float DummySpawnRadius = 800.f;

	UPROPERTY(EditDefaultsOnly, Category = "Lab")
	TArray<TSubclassOf<UGameplayAbility>> LabAbilitiesToGrant;

	UPROPERTY(EditDefaultsOnly, Category = "Lab")
	float GrantAbilitiesDelay = 1.5f;

	void SpawnLabDummies();
	void ScheduleGrantLabAbilities(APlayerController* PC);

	UFUNCTION()
	void GrantLabAbilitiesToPlayer(APlayerController* PC);
};
