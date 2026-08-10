// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AuraCharacterBase.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AuraGameplayTags.h"
#include "Lab/AuraLabDeveloperSettings.h"
#include "Lab/AuraLabLog.h"
#include "Lab/GA_Lab_PredictKeys.h"

AAuraCharacterBase::AAuraCharacterBase()
{
	//允许CharacterActor执行Tick函数
	PrimaryActorTick.bCanEverTick = true;

	//创建堆上的武器组件，并将其附加到骨骼网格组件的武器插槽上
	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon"));
	Weapon->SetupAttachment(GetMesh(), FName("WeaponHandSocket"));
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);


	//设置胶囊体和骨骼的碰撞预设
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECollisionResponse::ECR_Ignore);


	AttributeInitDataAsset = NewObject<UCharacterClassInfo>();

}

UAbilitySystemComponent* AAuraCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent.Get();
}

void AAuraCharacterBase::ShowCharacterAttribute()
{
	if (UAuraAttributeSet* AuraAttributeSet = Cast<UAuraAttributeSet>(AttributeSet.Get()))
	{
		UE_LOG(LogTemp, Log, TEXT("Health is %f"), AuraAttributeSet->GetHealth());
	}
}

void AAuraCharacterBase::Die()
{
	UE_LOG(LogTemp, Log, TEXT("%s has died."), *GetName());
	MulticastOnDeath();
}

FVector AAuraCharacterBase::GetCombatSocketLocation_Implementation(const FGameplayTag& SocketTag)
{
	// 取到全局 GameplayTag 单例，便于下面按 tag 名匹配攻击类型
	const FAuraGameplayTags& InputTags = FAuraGameplayTags::GetSingletonInstance();

	// 武器攻击：从武器骨骼网格组件上的「武器尖端」槽取位置
	if (SocketTag.MatchesTag(InputTags.Montage_Attack_Weapon))
	{
		check(Weapon);
		return Weapon->GetSocketLocation(WeaponTipSocketName);
	}

	// 左手攻击：从角色主骨骼网格体上的 LeftHand 槽取位置
	if (SocketTag.MatchesTag(InputTags.Montage_Attack_LeftHand))
	{
		return GetMesh()->GetSocketLocation(FName("LeftHand"));
	}

	// 右手攻击：从角色主骨骼网格体上的 RightHand 槽取位置
	if (SocketTag.MatchesTag(InputTags.Montage_Attack_RightHand))
	{
		return GetMesh()->GetSocketLocation(FName("RightHand"));
	}

	// 没有匹配到任何攻击类型时，回退到武器尖端槽，避免返回零向量导致技能逻辑异常
	check(Weapon);
	return Weapon->GetSocketLocation(WeaponTipSocketName);
}

bool AAuraCharacterBase::IsDead_Implementation() const
{
	return bDead;
}

AActor* AAuraCharacterBase::GetAvatar_Implementation()
{
	return this;
}

void AAuraCharacterBase::MulticastOnDeath_Implementation()
{
	if (IsValid(Weapon)) {
		Weapon->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));
		Weapon->SetSimulatePhysics(true);
		Weapon->SetEnableGravity(true);
		Weapon->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	}
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetEnableGravity(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Dissolve();

	// 玩家角色没有 HealthBar 组件，仅对有效对象隐藏血条
	if (IsValid(HealthBar))
	{
		HealthBar->SetVisibility(false);
	}
	bDead = true;
}

void AAuraCharacterBase::BindAttributeChangeDelegate()
{

}

void AAuraCharacterBase::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	
}

void AAuraCharacterBase::OnMaxHealthChanged(const FOnAttributeChangeData& Data)
{
}

void AAuraCharacterBase::GetHitByFire()
{

}

UAnimMontage* AAuraCharacterBase::GetHitReactAnimationMontage_Implementation()
{
	return HitReactMontage;
}

void AAuraCharacterBase::Dissolve()
{
	if(IsValid(DissolveMaterialInstance))
	{
		UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicMaterial);
		StartDissolveTimeline(DynamicMaterial);
	}
	if(IsValid(WeaponDissolveMaterialInstance))
	{
		UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(WeaponDissolveMaterialInstance, this);
		Weapon->SetMaterial(0, DynamicMaterial);
		StartWeaponDissolveTimeline(DynamicMaterial);
	}
}

void AAuraCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void AAuraCharacterBase::InitAbilityActorInfo()
{

}

void AAuraCharacterBase::InitializePrimaryAttributes()const
{
	check(IsValid(GetAbilitySystemComponent()));
	check(DefaultPrimaryAttributes);
	FGameplayEffectContextHandle EffectContextHandle = AbilitySystemComponent->MakeEffectContext();
	FGameplayEffectSpecHandle NewHandle = AbilitySystemComponent->MakeOutgoingSpec(DefaultPrimaryAttributes, 1, EffectContextHandle);
	GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(*NewHandle.Data.Get(), GetAbilitySystemComponent());

}

void AAuraCharacterBase::ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const
{
	check(IsValid(GetAbilitySystemComponent()));
	check(GameplayEffectClass);
	FGameplayEffectContextHandle EffectContextHandle = AbilitySystemComponent->MakeEffectContext();
	EffectContextHandle.AddSourceObject(this);
	FGameplayEffectSpecHandle NewHandle = AbilitySystemComponent->MakeOutgoingSpec(GameplayEffectClass, Level, EffectContextHandle);
	GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(*NewHandle.Data.Get(), GetAbilitySystemComponent());
	UE_LOG(LogTemp, Log, TEXT("%s Apply Effect To Self: %s"), *GameplayEffectClass->GetName(), *GetName());

}

void AAuraCharacterBase::InitializeDefaultAttributes() const
{
	ApplyEffectToSelf(DefaultPrimaryAttributes);
	ApplyEffectToSelf(DefaultSecondaryAttributes);
	ApplyEffectToSelf(DefaultVitalAttributes);
}

void AAuraCharacterBase::AddCharacterAbilities()
{
	UAuraAbilitySystemComponent* ASC = CastChecked<UAuraAbilitySystemComponent>(AbilitySystemComponent);
	ASC->AddCharacterAbilities(CharacterAbilities);

	// Lab：自动授予预测探针并绑 InputTag.4（Tag 必须在运行时取，不能信 CDO 构造期赋值）
	if (!HasAuthority())
	{
		return;
	}
	if (const UAuraLabDeveloperSettings* Settings = UAuraLabDeveloperSettings::Get())
	{
		if (!Settings->bLabModeEnabled)
		{
			return;
		}
	}

	const FGameplayTag Input4 = FAuraGameplayTags::GetSingletonInstance().InputTag_4;
	if (!Input4.IsValid())
	{
		AURA_LAB_LOG(Warning, TEXT("PredictKeys Grant 失败：InputTag.4 无效（NativeTags 未初始化？）"));
		return;
	}

	// 已授予则跳过，避免 InitAbilityActorInfo 重复 Give
	for (const FGameplayAbilitySpec& Existing : ASC->GetActivatableAbilities())
	{
		if (Existing.Ability && Existing.Ability->GetClass() == UGA_Lab_PredictKeys::StaticClass())
		{
			return;
		}
	}

	FGameplayAbilitySpec Spec(UGA_Lab_PredictKeys::StaticClass(), 1);
	Spec.DynamicAbilityTags.AddTag(Input4);
	ASC->GiveAbility(Spec);
	AURA_LAB_LOG(Warning, TEXT("Granted GA_Lab_PredictKeys | InputTag=%s — 按数字键 4 激活"), *Input4.ToString());
}

void AAuraCharacterBase::InitAttributeByCharacterInfo()
{
	if (AttributeInitDataAsset && CharacterType != ECharacterClass::None)
	{
		DefaultPrimaryAttributes = AttributeInitDataAsset->GetDefaultInfoForClass(CharacterType).PrimaryAttributeInitEffect;
		DefaultSecondaryAttributes = AttributeInitDataAsset->SecondaryAttributeInitEffect;
		DefaultVitalAttributes = AttributeInitDataAsset->VitalAttributeInitEffect;
	}
	else {
		if(CharacterType == ECharacterClass::None)
		{
			UE_LOG(LogTemp, Warning, TEXT("CharacterType is None in %s"), *GetName());
		}
		if(!AttributeInitDataAsset)
		{
			UE_LOG(LogTemp, Warning, TEXT("AttributeInitDataAsset is not set in %s"), *GetName());
		}
	}
}


