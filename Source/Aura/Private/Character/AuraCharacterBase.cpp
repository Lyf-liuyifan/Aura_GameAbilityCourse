// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AuraCharacterBase.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"

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
	Weapon->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));
	Weapon->SetSimulatePhysics(true);
	Weapon->SetEnableGravity(true);
	Weapon->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
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


