// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/AuraProjectilesSpell.h"
#include "Actor/AuraProjectile.h"
#include "Interaction/CombatInterface.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAbilityTypes.h"

void UAuraProjectilesSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
}

void UAuraProjectilesSpell::CastFireBolt(const FVector& ProjectileTargetLocation)
{
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
	if (!bIsServer) return;

	//服务器继续运行

	FTransform SpawnTransform;
	ICombatInterface* GetSpawnLocation = Cast<ICombatInterface>(GetAvatarActorFromActorInfo());
	if (GetSpawnLocation)
	{
		const FVector SocketLocation = GetSpawnLocation->GetCombatSocketLocation();
		FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation();
		Rotation.Pitch = 0.f;
		SpawnTransform.SetLocation(SocketLocation);
		SpawnTransform.SetRotation(Rotation.Quaternion());
	}
	AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(ProjectileClass,
		SpawnTransform,
		GetOwningActorFromActorInfo(),
		Cast<APawn>(GetOwningActorFromActorInfo()),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo());
	FAuraGameplayTags GameplayTags = FAuraGameplayTags::GetSingletonInstance();
	FGameplayEffectContextHandle EffectContextHandle = SourceASC->MakeEffectContext();
	EffectContextHandle.AddSourceObject(Projectile);
	FAuraGameplayEffectContext* AuraContext = FAuraGameplayEffectContext::ExtractEffectContext(EffectContextHandle);
	AuraContext->SetDamageType(GameplayTags.Damage_Fire);
	auto DamageSpec = SourceASC->MakeOutgoingSpec(DamageEffect, 1.0f, EffectContextHandle);
	const float DamageValue = Damage.GetValueAtLevel(GetAbilityLevel());
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(DamageSpec, GameplayTags.Damage, DamageValue);
	Projectile->DamageHandle = DamageSpec;	

	//发射物携带了Handle,发射物和目标撞击时触发Effect
	Projectile->FinishSpawning(SpawnTransform);
	UE_LOG(LogTemp, Log, TEXT("Activate GA_ProjectileSpell"));
}
