#include "Lab/AuraLabTargetDummy.h"

#include "Lab/AuraLabLog.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Components/CapsuleComponent.h"

AAuraLabTargetDummy::AAuraLabTargetDummy()
{
	AbilitySystemComponent = CreateDefaultSubobject<UAuraAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	AttributeSet = CreateDefaultSubobject<UAuraAttributeSet>(TEXT("AttributeSet"));

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}

void AAuraLabTargetDummy::BeginPlay()
{
	Super::BeginPlay();
	InitAbilityActorInfo();
	AURA_LAB_LOG(Log, TEXT("LabTargetDummy spawned | RepMode=Minimal | %s"), *GetName());
}

void AAuraLabTargetDummy::InitAbilityActorInfo()
{
	if (!AbilitySystemComponent) return;

	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	if (UAuraAbilitySystemComponent* AuraASC = Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent))
	{
		AuraASC->AbilityActorInfoSet();
	}

	InitAttributeByCharacterInfo();
	InitializeDefaultAttributes();
}
