#include "Lab/GA_Lab_MetaDamage.h"

#include "Lab/AuraLabDeveloperSettings.h"
#include "Lab/AuraLabLog.h"
#include "Lab/AuraLabTargetDummy.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AuraGameplayTags.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

UGA_Lab_MetaDamage::UGA_Lab_MetaDamage()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
}

void UGA_Lab_MetaDamage::ActivateAbility_Lab(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar || !Avatar->HasAuthority())
	{
		AURA_LAB_LOG(Warning, TEXT("GA_Lab_MetaDamage: damage applies on Server only"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	UAbilitySystemComponent* SourceASC = ActorInfo->AbilitySystemComponent.Get();
	AAuraLabTargetDummy* Dummy = FindNearestDummy(Avatar);
	if (!SourceASC || !Dummy)
	{
		AURA_LAB_LOG(Warning, TEXT("GA_Lab_MetaDamage: no Source ASC or Lab dummy in level"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilitySystemComponent* TargetASC = Dummy->GetAbilitySystemComponent();
	if (!TargetASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	TSubclassOf<UGameplayEffect> EffectClass = DamageEffectClass;
	if (!EffectClass)
	{
		if (const UAuraLabDeveloperSettings* Settings = UAuraLabDeveloperSettings::Get())
		{
			EffectClass = Settings->DefaultDamageEffectClass.LoadSynchronous();
		}
	}

	if (!EffectClass)
	{
		AURA_LAB_LOG(Error, TEXT("GA_Lab_MetaDamage: set DamageEffectClass to GE_Damage"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const FAuraGameplayTags& Tags = FAuraGameplayTags::GetSingletonInstance();
	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddSourceObject(this);
	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(EffectClass, GetAbilityLevel(), Context);
	if (SpecHandle.IsValid())
	{
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, Tags.Damage, LabDamageMagnitude);
		SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
		AURA_LAB_LOG(Warning, TEXT("Applied damage GE to %s magnitude=%.1f"), *Dummy->GetName(), LabDamageMagnitude);
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

AAuraLabTargetDummy* UGA_Lab_MetaDamage::FindNearestDummy(const AActor* SourceActor) const
{
	if (!SourceActor || !GetWorld()) return nullptr;

	AAuraLabTargetDummy* Nearest = nullptr;
	float BestDistSq = MAX_FLT;
	for (TActorIterator<AAuraLabTargetDummy> It(GetWorld()); It; ++It)
	{
		const float DistSq = FVector::DistSquared(SourceActor->GetActorLocation(), It->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Nearest = *It;
		}
	}
	return Nearest;
}
