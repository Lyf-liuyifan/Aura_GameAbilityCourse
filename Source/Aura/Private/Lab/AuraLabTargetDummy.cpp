#include "Lab/AuraLabTargetDummy.h"

#include "Lab/AuraLabLog.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"

AAuraLabTargetDummy::AAuraLabTargetDummy()
{
	// 靶子需要参与网络复制（UE5 为成员变量，无 SetNetUpdateFrequency）
	bReplicates = true;
	NetUpdateFrequency = 100.f;

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

void AAuraLabTargetDummy::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAuraLabTargetDummy, LabHitCount);
	DOREPLIFETIME(AAuraLabTargetDummy, LabPayload);
}

void AAuraLabTargetDummy::ApplyServerHit()
{
	// 仅权威端修改复制属性
	if (!HasAuthority())
	{
		return;
	}

	++LabHitCount;
	AURA_LAB_LOG(Warning, TEXT("ApplyServerHit | %s HitCount=%d"), *GetName(), LabHitCount);
	MulticastPlayHitFX();
}

void AAuraLabTargetDummy::MulticastPlayHitFX_Implementation()
{
	// 所有端播放占位特效：缩放脉冲表示受击
	const FVector Pulse(1.1f, 1.1f, 1.1f);
	SetActorScale3D(Pulse);
	AURA_LAB_LOG(Log, TEXT("MulticastPlayHitFX | %s"), *GetName());
}

void AAuraLabTargetDummy::SetLabPayload(const FLabNetSerializePayload& NewPayload)
{
	if (!HasAuthority())
	{
		return;
	}
	LabPayload = NewPayload;
}

void AAuraLabTargetDummy::OnRep_LabHitCount()
{
	AURA_LAB_LOG(Warning, TEXT("OnRep_LabHitCount | %s HitCount=%d"), *GetName(), LabHitCount);
}

void AAuraLabTargetDummy::OnRep_LabPayload()
{
	AURA_LAB_LOG(Warning, TEXT("OnRep_LabPayload | %s BuffId=%d Magnitude=%.2f"),
		*GetName(), LabPayload.BuffId, LabPayload.Magnitude);
}
