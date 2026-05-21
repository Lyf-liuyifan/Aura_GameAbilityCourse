// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/AuraProjectilesSpell.h"
#include "Actor/AuraProjectile.h"
#include "Interaction/CombatInterface.h"

void UAuraProjectilesSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	const bool bIsServer = HasAuthority(&ActivationInfo);
	if (!bIsServer) return;


	FTransform SpawnTransform;
	ICombatInterface* GetSpawnLocation = Cast<ICombatInterface>(GetAvatarActorFromActorInfo());
	if (GetSpawnLocation)
	{
		const FVector SocketLocation = GetSpawnLocation->GetCombatSocketLocation();
		SpawnTransform.SetLocation(SocketLocation);
	}
	AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(ProjectileClass, 
		SpawnTransform,
		GetOwningActorFromActorInfo(), 
		Cast<APawn>(GetOwningActorFromActorInfo()),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	Projectile->FinishSpawning(SpawnTransform);
	UE_LOG(LogTemp, Log, TEXT("Activate GA_ProjectileSpell"));
}
