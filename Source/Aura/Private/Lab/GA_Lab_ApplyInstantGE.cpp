#include "Lab/GA_Lab_ApplyInstantGE.h"

#include "Lab/AuraLabDeveloperSettings.h"
#include "Lab/AuraLabLog.h"
#include "AbilitySystemComponent.h"

UGA_Lab_ApplyInstantGE::UGA_Lab_ApplyInstantGE()
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UGA_Lab_ApplyInstantGE::ActivateAbility_Lab(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (!ASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	TSubclassOf<UGameplayEffect> EffectClass = InstantEffectClass;
	if (!EffectClass)
	{
		if (const UAuraLabDeveloperSettings* Settings = UAuraLabDeveloperSettings::Get())
		{
			EffectClass = Settings->DefaultInstantEffectClass.LoadSynchronous();
		}
	}

	if (!EffectClass)
	{
		AURA_LAB_LOG(Error, TEXT("GA_Lab_ApplyInstantGE: set InstantEffectClass in BP defaults or DeveloperSettings"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	Context.AddSourceObject(this);
	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(EffectClass, GetAbilityLevel(), Context);
	if (SpecHandle.IsValid())
	{
		ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		AURA_LAB_LOG(Warning, TEXT("Applied Instant GE %s to self"), *EffectClass->GetName());
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
