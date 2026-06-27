#include "Lab/AuraLabGameMode.h"

#include "Lab/AuraLabDeveloperSettings.h"
#include "Lab/AuraLabLibrary.h"
#include "Lab/AuraLabLog.h"
#include "Lab/AuraLabTargetDummy.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AAuraLabGameMode::AAuraLabGameMode()
{
}

void AAuraLabGameMode::StartPlay()
{
	Super::StartPlay();

	if (HasAuthority())
	{
		SpawnLabDummies();
	}
}

void AAuraLabGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	ScheduleGrantLabAbilities(NewPlayer);
}

void AAuraLabGameMode::SpawnLabDummies()
{
	if (!TargetDummyClass)
	{
		if (const UAuraLabDeveloperSettings* Settings = UAuraLabDeveloperSettings::Get())
		{
			TargetDummyClass = Settings->DefaultTargetDummyClass.LoadSynchronous();
		}
	}

	if (!TargetDummyClass)
	{
		AURA_LAB_LOG(Warning, TEXT("AuraLabGameMode: set TargetDummyClass on BP_AuraLabGameMode"));
		return;
	}

	APawn* ReferencePawn = UGameplayStatics::GetPlayerPawn(this, 0);
	FVector Origin = ReferencePawn ? ReferencePawn->GetActorLocation() : FVector::ZeroVector;

	for (int32 i = 0; i < DummyCount; ++i)
	{
		const float Angle = (2.f * PI * i) / FMath::Max(DummyCount, 1);
		const FVector Offset(FMath::Cos(Angle) * DummySpawnRadius, FMath::Sin(Angle) * DummySpawnRadius, 0.f);
		const FVector SpawnLocation = Origin + Offset;

		GetWorld()->SpawnActor<AAuraLabTargetDummy>(TargetDummyClass, SpawnLocation, FRotator::ZeroRotator);
	}

	AURA_LAB_LOG(Log, TEXT("Spawned %d lab dummies"), DummyCount);
}

void AAuraLabGameMode::ScheduleGrantLabAbilities(APlayerController* PC)
{
	if (!PC || !HasAuthority()) return;

	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(
		TimerHandle,
		FTimerDelegate::CreateUObject(this, &AAuraLabGameMode::GrantLabAbilitiesToPlayer, PC),
		GrantAbilitiesDelay,
		false);
}

void AAuraLabGameMode::GrantLabAbilitiesToPlayer(APlayerController* PC)
{
	TArray<TSubclassOf<UGameplayAbility>> Abilities = LabAbilitiesToGrant;
	if (Abilities.Num() == 0)
	{
		if (const UAuraLabDeveloperSettings* Settings = UAuraLabDeveloperSettings::Get())
		{
			Abilities = Settings->DefaultLabAbilities;
		}
	}

	if (Abilities.Num() == 0)
	{
		AURA_LAB_LOG(Warning, TEXT("No Lab abilities configured — add to GameMode or DeveloperSettings"));
		return;
	}

	UAuraLabLibrary::GrantLabAbilities(this, PC, Abilities);
}
