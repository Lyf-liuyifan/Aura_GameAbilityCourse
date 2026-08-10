#include "Lab/AuraLabLibrary.h"

#include "Lab/AuraLabDeveloperSettings.h"
#include "Lab/AuraLabLog.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Abilities/GameplayAbility.h"
#include "Engine/Engine.h"
#include "Engine/NetConnection.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Pawn.h"
#include "HAL/IConsoleManager.h"
#include "Player/AuraPlayerController.h"
#include "Player/AuraPlayerState.h"
#include "UObject/UObjectIterator.h"

namespace AuraLabConsole
{
	static TAutoConsoleVariable<int32> CVarWrongGiveAbility(
		TEXT("AuraLab.WrongGiveAbility"),
		0,
		TEXT("When 1, SimulateWrongClientGiveAbility grants on client only (Lab N04)."),
		ECVF_Default);
}

static UAbilitySystemComponent* ResolveASC(APlayerController* PC)
{
	if (!PC) return nullptr;
	if (AAuraPlayerState* PS = PC->GetPlayerState<AAuraPlayerState>())
	{
		return PS->GetAbilitySystemComponent();
	}
	if (APawn* Pawn = PC->GetPawn())
	{
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Pawn))
		{
			return ASI->GetAbilitySystemComponent();
		}
	}
	return nullptr;
}

void UAuraLabLibrary::LogNetRole(AActor* Actor)
{
	if (!Actor)
	{
		AURA_LAB_LOG(Warning, TEXT("LogNetRole: Actor is null"));
		return;
	}

	const bool bLocallyControlled = Cast<APawn>(Actor) ? Cast<APawn>(Actor)->IsLocallyControlled() : false;
	AURA_LAB_LOG(Warning, TEXT("NetRole %s | LocalRole=%d RemoteRole=%d Authority=%d LocallyControlled=%d"),
		*Actor->GetName(),
		static_cast<int32>(Actor->GetLocalRole()),
		static_cast<int32>(Actor->GetRemoteRole()),
		Actor->HasAuthority(),
		bLocallyControlled);
}

void UAuraLabLibrary::LogASCSummary(UAbilitySystemComponent* ASC)
{
	if (!ASC)
	{
		AURA_LAB_LOG(Warning, TEXT("LogASC: ASC is null"));
		return;
	}

	const AActor* Owner = ASC->GetOwnerActor();
	const AActor* Avatar = ASC->GetAvatarActor();
	FString RepMode = TEXT("Unknown");
	switch (ASC->ReplicationMode)
	{
	case EGameplayEffectReplicationMode::Minimal: RepMode = TEXT("Minimal"); break;
	case EGameplayEffectReplicationMode::Mixed: RepMode = TEXT("Mixed"); break;
	case EGameplayEffectReplicationMode::Full: RepMode = TEXT("Full"); break;
	default: break;
	}

	AURA_LAB_LOG(Warning, TEXT("ASC Summary | Owner=%s Avatar=%s RepMode=%s Abilities=%d ActiveGE=%d"),
		*GetNameSafe(Owner),
		*GetNameSafe(Avatar),
		*RepMode,
		ASC->GetActivatableAbilities().Num(),
		ASC->GetActiveGameplayEffects().GetAllActiveEffectHandles().Num());
}

void UAuraLabLibrary::LogActivationInfo(AActor* AvatarActor, const FGameplayAbilityActivationInfo& ActivationInfo, const FString& AbilityName)
{
	const TCHAR* ModeStr = TEXT("Unknown");
	switch (ActivationInfo.ActivationMode)
	{
	case EGameplayAbilityActivationMode::Authority: ModeStr = TEXT("Authority"); break;
	case EGameplayAbilityActivationMode::NonAuthority: ModeStr = TEXT("NonAuthority"); break;
	case EGameplayAbilityActivationMode::Predicting: ModeStr = TEXT("Predicting"); break;
	case EGameplayAbilityActivationMode::Confirmed: ModeStr = TEXT("Confirmed"); break;
	case EGameplayAbilityActivationMode::Rejected: ModeStr = TEXT("Rejected"); break;
	default: break;
	}

	const FPredictionKey& Key = ActivationInfo.GetActivationPredictionKey();
	AURA_LAB_LOG(Warning, TEXT("Activate %s | Mode=%s Key=%d Auth=%d Avatar=%s"),
		*AbilityName,
		ModeStr,
		Key.Current,
		AvatarActor ? AvatarActor->HasAuthority() : false,
		*GetNameSafe(AvatarActor));
}

UAbilitySystemComponent* UAuraLabLibrary::GetLabASCFromController(APlayerController* PC)
{
	return ResolveASC(PC);
}

void UAuraLabLibrary::GrantLabAbilities(UObject* WorldContextObject, APlayerController* PC, const TArray<TSubclassOf<UGameplayAbility>>& Abilities)
{
	UAbilitySystemComponent* ASC = ResolveASC(PC);
	if (!ASC || !PC)
	{
		AURA_LAB_LOG(Warning, TEXT("GrantLabAbilities: no ASC for controller（先确认已 Possess 角色且 PlayerState 是 AuraPlayerState）"));
		return;
	}

	if (!PC->HasAuthority())
	{
		AURA_LAB_LOG(Warning, TEXT("GrantLabAbilities: must run on Server (Authority)。多人大厅请在主机窗口执行 AuraLab.GrantAll"));
		return;
	}

	// 统计实际授予数量，避免 Settings 里类指针为空时静默跳过
	int32 GrantedCount = 0;
	int32 SkippedNull = 0;
	for (const TSubclassOf<UGameplayAbility>& AbilityClass : Abilities)
	{
		if (!AbilityClass)
		{
			++SkippedNull;
			continue;
		}

		FGameplayAbilitySpec Spec(AbilityClass, 1, INDEX_NONE, ASC->GetOwner());
		ASC->GiveAbility(Spec);
		++GrantedCount;
		AURA_LAB_LOG(Warning, TEXT("Granted Lab ability: %s"), *AbilityClass->GetName());
	}

	AURA_LAB_LOG(Warning, TEXT("GrantLabAbilities done | Granted=%d SkippedNull=%d Input=%d ActivatableNow=%d"),
		GrantedCount, SkippedNull, Abilities.Num(), ASC->GetActivatableAbilities().Num());
}

bool UAuraLabLibrary::ActivateLabAbilityByClassName(UObject* WorldContextObject, APlayerController* PC, FName AbilityClassName)
{
	UAbilitySystemComponent* ASC = ResolveASC(PC);
	if (!ASC || !PC)
	{
		AURA_LAB_LOG(Warning, TEXT("ActivateLabAbility: no ASC"));
		return false;
	}

	const FString TargetName = AbilityClassName.ToString();

	auto TryActivateMatching = [&]() -> bool
	{
		for (FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
		{
			if (Spec.Ability && Spec.Ability->GetClass()->GetName().Contains(TargetName))
			{
				const bool bActivated = ASC->TryActivateAbility(Spec.Handle);
				AURA_LAB_LOG(Warning, TEXT("TryActivate %s -> %s"), *Spec.Ability->GetClass()->GetName(), bActivated ? TEXT("OK") : TEXT("FAILED"));
				return bActivated;
			}
		}
		return false;
	};

	if (TryActivateMatching())
	{
		return true;
	}

	// 未 Grant 时：主机上按类名补发一次，避免「只改了 Project Settings 却忘了 GrantAll」
	if (PC->HasAuthority())
	{
		UClass* FoundClass = nullptr;
		for (TObjectIterator<UClass> It; It; ++It)
		{
			if (!It->IsChildOf(UGameplayAbility::StaticClass()) || It->HasAnyClassFlags(CLASS_Abstract))
			{
				continue;
			}
			if (It->GetName().Contains(TargetName))
			{
				FoundClass = *It;
				break;
			}
		}

		if (FoundClass)
		{
			FGameplayAbilitySpec Spec(FoundClass, 1, INDEX_NONE, ASC->GetOwner());
			ASC->GiveAbility(Spec);
			AURA_LAB_LOG(Warning, TEXT("ActivateLabAbility: auto-granted %s then retry"), *FoundClass->GetName());
			if (TryActivateMatching())
			{
				return true;
			}
		}
	}
	else
	{
		AURA_LAB_LOG(Warning, TEXT("ActivateLabAbility: ASC 上没有 '%s'，且当前不是 Authority。请在主机执行 AuraLab.GrantAll 后再 Activate"), *TargetName);
	}

	// 打印现有技能，方便对照名字是否写错
	AURA_LAB_LOG(Warning, TEXT("ActivateLabAbility: no ability matching '%s'. Activatable=%d"), *TargetName, ASC->GetActivatableAbilities().Num());
	for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		if (Spec.Ability)
		{
			AURA_LAB_LOG(Warning, TEXT("  - %s"), *Spec.Ability->GetClass()->GetName());
		}
	}
	return false;
}

void UAuraLabLibrary::SimulateWrongClientGiveAbility(APlayerController* PC, TSubclassOf<UGameplayAbility> AbilityClass)
{
	if (!PC || !AbilityClass)
	{
		AURA_LAB_LOG(Warning, TEXT("SimulateWrongClientGiveAbility: invalid args"));
		return;
	}

	if (PC->HasAuthority())
	{
		AURA_LAB_LOG(Warning, TEXT("SimulateWrongClientGiveAbility: run this on Client PIE window (non-Authority)"));
		return;
	}

	UAbilitySystemComponent* ASC = ResolveASC(PC);
	if (!ASC) return;

	FGameplayAbilitySpec Spec(AbilityClass, 1, INDEX_NONE, ASC->GetOwner());
	ASC->GiveAbility(Spec);
	AURA_LAB_LOG(Warning, TEXT("WRONG: Client-side GiveAbility %s — other clients will NOT see this Spec"), *AbilityClass->GetName());
}

void UAuraLabLibrary::RegisterConsoleCommands()
{
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("AuraLab.LogNetRole"),
		TEXT("Log network role of local pawn."),
		FConsoleCommandDelegate::CreateLambda([]()
		{
			if (GEngine && GEngine->GameViewport)
			{
				if (UWorld* World = GEngine->GameViewport->GetWorld())
				{
					if (APlayerController* PC = World->GetFirstPlayerController())
					{
						LogNetRole(PC->GetPawn());
					}
				}
			}
		}));

	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("AuraLab.LogASC"),
		TEXT("Log ASC summary for local player."),
		FConsoleCommandDelegate::CreateLambda([]()
		{
			if (GEngine && GEngine->GameViewport)
			{
				if (UWorld* World = GEngine->GameViewport->GetWorld())
				{
					if (APlayerController* PC = World->GetFirstPlayerController())
					{
						LogASCSummary(ResolveASC(PC));
					}
				}
			}
		}));

	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("AuraLab.GrantAll"),
		TEXT("Grant all Lab abilities from DeveloperSettings. Client 会自动 Server RPC。"),
		FConsoleCommandDelegate::CreateLambda([]()
		{
			if (!GEngine || !GEngine->GameViewport) return;
			UWorld* World = GEngine->GameViewport->GetWorld();
			if (!World) return;
			APlayerController* PC = World->GetFirstPlayerController();
			if (!PC) return;

			TArray<TSubclassOf<UGameplayAbility>> Abilities;
			if (const UAuraLabDeveloperSettings* Settings = UAuraLabDeveloperSettings::Get())
			{
				Abilities = Settings->DefaultLabAbilities;
				AURA_LAB_LOG(Warning, TEXT("AuraLab.GrantAll | Settings DefaultLabAbilities=%d Authority=%d"),
					Abilities.Num(), PC->HasAuthority());
			}
			else
			{
				AURA_LAB_LOG(Warning, TEXT("AuraLab.GrantAll: AuraLabDeveloperSettings missing"));
				return;
			}

			// 多人 PIE 时 Output Log 常打在 Client：改为 RPC，避免必须点到 Server 窗口
			if (!PC->HasAuthority())
			{
				if (AAuraPlayerController* AuraPC = Cast<AAuraPlayerController>(PC))
				{
					AuraPC->Server_GrantLabAbilities();
					AURA_LAB_LOG(Warning, TEXT("AuraLab.GrantAll: 已从 Client 转发 Server RPC，稍后看 Granted 日志再 Activate"));
					return;
				}

				AURA_LAB_LOG(Warning, TEXT("AuraLab.GrantAll: 当前 PC 不是 AuraPlayerController，无法 RPC。请用单人 PIE 或主机窗口"));
				return;
			}

			GrantLabAbilities(World, PC, Abilities);
		}));

	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("AuraLab.Activate"),
		TEXT("AuraLab.Activate GA_Lab_PredictOnly — activate lab ability by partial class name."),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			if (Args.Num() < 1) return;
			if (!GEngine || !GEngine->GameViewport) return;
			UWorld* World = GEngine->GameViewport->GetWorld();
			if (!World) return;
			APlayerController* PC = World->GetFirstPlayerController();
			ActivateLabAbilityByClassName(World, PC, FName(*Args[0]));
		}));

	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("AuraLab.SimulateWrongClientGiveAbility"),
		TEXT("Client-only wrong GiveAbility demo. Optional arg: ability class name substring."),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			if (!GEngine || !GEngine->GameViewport) return;
			UWorld* World = GEngine->GameViewport->GetWorld();
			if (!World) return;
			APlayerController* PC = World->GetFirstPlayerController();
			TSubclassOf<UGameplayAbility> AbilityClass = UGameplayAbility::StaticClass();
			if (Args.Num() > 0)
			{
				for (TObjectIterator<UClass> It; It; ++It)
				{
					if (It->IsChildOf(UGameplayAbility::StaticClass()) && It->GetName().Contains(Args[0]))
					{
						AbilityClass = *It;
						break;
					}
				}
			}
			SimulateWrongClientGiveAbility(PC, AbilityClass);
		}));

	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("AuraLab.NetDump"),
		TEXT("Log network connection summary for local player."),
		FConsoleCommandDelegate::CreateLambda([]()
		{
			if (!GEngine || !GEngine->GameViewport) return;
			UWorld* World = GEngine->GameViewport->GetWorld();
			if (!World) return;
			APlayerController* PC = World->GetFirstPlayerController();
			if (!PC) return;

			UNetConnection* Conn = PC->GetNetConnection();
			AURA_LAB_LOG(Warning, TEXT("NetDump | PC=%s Authority=%d Conn=%s"),
				*PC->GetName(),
				PC->HasAuthority(),
				Conn ? *Conn->GetName() : TEXT("None"));

			if (Conn)
			{
				float PingMs = 0.f;
				if (APlayerState* PS = PC->PlayerState)
				{
					PingMs = PS->ExactPing;
				}
				AURA_LAB_LOG(Warning, TEXT("NetDump | Ping=%.1fms Driver=%s"),
					PingMs,
					Conn->Driver ? *Conn->Driver->GetName() : TEXT("None"));
			}
		}));

	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("AuraLab.SimulateLag"),
		TEXT("AuraLab.SimulateLag <ms> <lossPercent> — set net pktlag and pktloss."),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			const int32 LagMs = Args.Num() > 0 ? FCString::Atoi(*Args[0]) : 0;
			const int32 Loss = Args.Num() > 1 ? FCString::Atoi(*Args[1]) : 0;

			if (IConsoleVariable* LagVar = IConsoleManager::Get().FindConsoleVariable(TEXT("net pktlag")))
			{
				LagVar->Set(LagMs);
			}
			if (IConsoleVariable* LossVar = IConsoleManager::Get().FindConsoleVariable(TEXT("net pktloss")))
			{
				LossVar->Set(Loss);
			}
			AURA_LAB_LOG(Warning, TEXT("SimulateLag | pktlag=%d pktloss=%d"), LagMs, Loss);
		}));
}

static struct FAuraLabConsoleRegistrar
{
	FAuraLabConsoleRegistrar()
	{
		UAuraLabLibrary::RegisterConsoleCommands();
	}
} GAuraLabConsoleRegistrar;
