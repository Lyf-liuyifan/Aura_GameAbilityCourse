// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AuraCharacterBase.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
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

void AAuraCharacterBase::BindAttributeChangeDelegate()
{

}

void AAuraCharacterBase::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	
}

void AAuraCharacterBase::OnMaxHealthChanged(const FOnAttributeChangeData& Data)
{
}

void AAuraCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

FVector AAuraCharacterBase::GetCombatSocketLocation()
{
	return Weapon->GetSocketLocation(WeaponTipSocketName);
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

}

void AAuraCharacterBase::InitializeDefaultAbilities() const
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


