// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/AuraRangedAttack.h"
#include "Actor/AuraProjectile.h"
#include "Character/AuraCharacterBase.h"
#include "AbilitySystem/Data/AuraTagToMontage.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AuraGameplayTags.h"
#include "MotionWarpingComponent.h"

UAuraRangedAttack::UAuraRangedAttack()
{
	// AI 攻击只在服务器驱动；同一 Actor 复用实例，避免 InstancedPerExecution 叠多层
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	const FAuraGameplayTags& Tags = FAuraGameplayTags::GetSingletonInstance();
	DamageTypeTag = Tags.Damage_Physical;

	EventTag_RockSpawn = Tags.Event_Montage_Rock_Spawn;
	EventTag_RockLaunch = Tags.Event_Montage_Rock_Launch;
	AttackMontageTag = Tags.Abilities_Attack;
}

void UAuraRangedAttack::PostInitProperties()
{
	Super::PostInitProperties();

	// 蓝图 CDO 序列化可能覆盖构造函数里写入的 AbilityTriggers，因此在 CDO 初始化后再补一次
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		EnsureAbilityTriggersRegistered();
	}
}

void UAuraRangedAttack::EnsureAbilityTriggersRegistered()
{
	// 优先用 Native Tag；若 PostInitProperties 早于 Tag 注册，则回退 RequestGameplayTag
	FGameplayTag RequiredTag = FAuraGameplayTags::GetSingletonInstance().Event_Attack_Ranged;
	if (!RequiredTag.IsValid())
	{
		RequiredTag = FGameplayTag::RequestGameplayTag(FName("Event.Attack.Ranged"), false);
	}
	if (!RequiredTag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[RangedAttack] EnsureAbilityTriggersRegistered: Event.Attack.Ranged tag not found"));
		return;
	}

	// 清除无效 Trigger（Tag 为空时 PostInit 可能误写入 None）
	for (int32 i = AbilityTriggers.Num() - 1; i >= 0; --i)
	{
		if (!AbilityTriggers[i].TriggerTag.IsValid()
			|| AbilityTriggers[i].TriggerSource != EGameplayAbilityTriggerSource::GameplayEvent)
		{
			AbilityTriggers.RemoveAt(i);
		}
	}

	for (const FAbilityTriggerData& Existing : AbilityTriggers)
	{
		if (Existing.TriggerTag == RequiredTag)
		{
			UE_LOG(LogTemp, Log, TEXT("[RangedAttack] %s: Trigger already registered: %s"),
				*GetName(), *RequiredTag.ToString());
			return;
		}
	}

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = RequiredTag;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
	UE_LOG(LogTemp, Log, TEXT("[RangedAttack] %s: Added AbilityTrigger %s (total=%d)"),
		*GetName(), *RequiredTag.ToString(), AbilityTriggers.Num());
}

bool UAuraRangedAttack::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		UE_LOG(LogTemp, Warning, TEXT("[RangedAttack] %s: CanActivate blocked by Super"),
			*GetNameSafe(ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr));
		return false;
	}

	// 已在播放攻击蒙太奇时拒绝重复激活，避免 Notify 链被打断
	if (const UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr)
	{
		for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
		{
			if (!Spec.IsActive() || !Spec.Ability) continue;

			if (Spec.Ability->GetClass()->IsChildOf(GetClass()))
			{
				UE_LOG(LogTemp, Log, TEXT("[RangedAttack] %s: CanActivate blocked — same class GA already active"),
					*GetNameSafe(ActorInfo->AvatarActor.Get()));
				return false;
			}
		}
	}

	return true;
}

void UAuraRangedAttack::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UE_LOG(LogTemp, Log, TEXT("[RangedAttack] %s: ActivateAbility (Authority=%d, EventTag=%s)"),
		*GetNameSafe(GetAvatarActorFromActorInfo()),
		ActorInfo && ActorInfo->IsNetAuthority(),
		TriggerEventData ? *TriggerEventData->EventTag.ToString() : TEXT("None"));

	// 1. 只在服务器执行 AI 攻击逻辑
	if (!ActorInfo->IsNetAuthority())
	{
		UE_LOG(LogTemp, Log, TEXT("[RangedAttack] %s: skipped on non-authority"), *GetNameSafe(GetAvatarActorFromActorInfo()));
		return;
	}

	// 2. 从 BTTask 传入的 EventData 解析玩家目标位置
	if (!ResolveTargetLocation(TriggerEventData, TargetLocation))
	{
		UE_LOG(LogTemp, Warning, TEXT("[RangedAttack] %s: Failed — invalid Target in EventData"),
			*GetNameSafe(GetAvatarActorFromActorInfo()));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 3. 查攻击蒙太奇
	AAuraCharacterBase* Character = Cast<AAuraCharacterBase>(GetAvatarActorFromActorInfo());
	UAuraTagToMontage* TagToMontage = Character ? Character->GetTagToMontageData() : nullptr;
	UAnimMontage* AttackMontage = TagToMontage
		? TagToMontage->GetMontageByTagFromData(Character, AttackMontageTag)
		: nullptr;

	if (!AttackMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("[RangedAttack] %s: Attack montage not found (Category=%d, Tag=%s, TagToMontage=%d)"),
			*GetNameSafe(Character),
			Character ? static_cast<int32>(Character->CharacterCategory) : -1,
			*AttackMontageTag.ToString(),
			TagToMontage != nullptr);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[RangedAttack] %s: Playing montage %s toward %s"),
		*GetNameSafe(Character), *GetNameSafe(AttackMontage), *TargetLocation.ToCompactString());

	//设置motion warping 目标
	if(UMotionWarpingComponent* MotionWarping = Character->FindComponentByClass<UMotionWarpingComponent>())
	{
		// 名称必须与 Montage 里 Motion Warping Notify 的 Warp Target Name 一致
		MotionWarping->AddOrUpdateWarpTargetFromLocation(
			FName("AttackDirectionWarping"),
			TargetLocation);
	}

	// 4. 并行：播攻击蒙太奇（Interrupted 也必须 EndAbility，否则 HitReact 打断后 GA 永不结束、BT 卡死）
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, AttackMontage);
	MontageTask->OnCompleted.AddDynamic(this, &UAuraRangedAttack::OnMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &UAuraRangedAttack::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UAuraRangedAttack::OnMontageCancelled);
	MontageTask->OnInterrupted.AddDynamic(this, &UAuraRangedAttack::OnMontageCancelled);
	MontageTask->ReadyForActivation();

	// 5. 并行：等 Notify1 —— 生成石头并 Attach 到皮兜
	UAbilityTask_WaitGameplayEvent* SpawnTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, EventTag_RockSpawn);
	SpawnTask->OnlyMatchExact = true;
	SpawnTask->EventReceived.AddDynamic(this, &UAuraRangedAttack::OnRockSpawnEvent);
	SpawnTask->ReadyForActivation();

	// 6. 并行：等 Notify2 —— 松手发射
	UAbilityTask_WaitGameplayEvent* LaunchTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, EventTag_RockLaunch);
	LaunchTask->OnlyMatchExact = true;
	LaunchTask->EventReceived.AddDynamic(this, &UAuraRangedAttack::OnRockLaunchEvent);
	LaunchTask->ReadyForActivation();
}

void UAuraRangedAttack::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	// 能力取消时销毁未发射的石头，避免挂在皮兜上残留
	if (HeldRock)
	{
		HeldRock->Destroy();
		HeldRock = nullptr;
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	UE_LOG(LogTemp, Log, TEXT("[RangedAttack] %s: EndAbility (Cancelled=%d)"),
		*GetNameSafe(ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr), bWasCancelled);
}

bool UAuraRangedAttack::ResolveTargetLocation(const FGameplayEventData* TriggerEventData, FVector& OutLocation) const
{
	if (!TriggerEventData) return false;

	// BTTask_Attack 把玩家 Actor 写在 EventData.Target
	const AActor* TargetActor = TriggerEventData->Target.Get();
	if (!IsValid(TargetActor)) return false;

	OutLocation = TargetActor->GetActorLocation();
	return true;
}

AAuraProjectile* UAuraRangedAttack::SpawnAndAttachHeldRock(AAuraCharacterBase* Character)
{
	if (!Character || !ProjectileClass) return nullptr;

	USkeletalMeshComponent* WeaponMesh = Character->GetWeaponMesh();
	if (!WeaponMesh) return nullptr;

	const FTransform SpawnTransform(
		WeaponMesh->GetSocketRotation(ProjectileAttachSocketName),
		WeaponMesh->GetSocketLocation(ProjectileAttachSocketName));

	AAuraProjectile* Rock = GetWorld()->SpawnActorDeferred<AAuraProjectile>(
		ProjectileClass,
		SpawnTransform,
		GetOwningActorFromActorInfo(),
		Cast<APawn>(GetAvatarActorFromActorInfo()),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	if (!Rock) return nullptr;

	// FinishSpawning 前进入握持，BeginPlay 会跳过飞行音效
	Rock->EnterHeldState();
	Rock->FinishSpawning(SpawnTransform);

	// Attach 到弹弓皮兜，后拉动画会自动带着石头动
	Rock->AttachToComponent(
		WeaponMesh,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		ProjectileAttachSocketName);

	return Rock;
}

void UAuraRangedAttack::OnRockSpawnEvent(FGameplayEventData Payload)
{
	// 防止重复 Spawn
	if (HeldRock) return;

	AAuraCharacterBase* Character = Cast<AAuraCharacterBase>(GetAvatarActorFromActorInfo());
	HeldRock = SpawnAndAttachHeldRock(Character);
	if (!HeldRock)
	{
		UE_LOG(LogTemp, Warning, TEXT("[RangedAttack] %s: Rock spawn failed (ProjectileClass=%d, WeaponMesh=%d)"),
			*GetNameSafe(Character), ProjectileClass != nullptr,
			Character && Character->GetWeaponMesh() != nullptr);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[RangedAttack] %s: Rock spawned and attached"), *GetNameSafe(Character));
	}
}

void UAuraRangedAttack::OnRockLaunchEvent(FGameplayEventData Payload)
{
	if (!HeldRock)
	{
		UE_LOG(LogTemp, Warning, TEXT("[RangedAttack] %s: Launch event but no HeldRock"),
			*GetNameSafe(GetAvatarActorFromActorInfo()));
		return;
	}

	// 松手时再挂伤害 Spec（与 CastFireBolt 同一套）
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	HeldRock->DamageHandle = MakeDamageEffectSpec(SourceASC, DamageEffect, HeldRock);

	UE_LOG(LogTemp, Log, TEXT("[RangedAttack] %s: Launch rock toward %s"),
		*GetNameSafe(GetAvatarActorFromActorInfo()), *TargetLocation.ToCompactString());
	HeldRock->LaunchToward(TargetLocation);
	HeldRock = nullptr;
}

void UAuraRangedAttack::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UAuraRangedAttack::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
