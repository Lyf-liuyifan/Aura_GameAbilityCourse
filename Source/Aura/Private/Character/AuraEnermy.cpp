// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AuraEnermy.h"
#include "AI/AuraAIController.h"
#include "../Aura.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "UI/Widget/AuraUserWidget.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Components/CapsuleComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"

void AAuraEnermy::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	if (!HasAuthority()) return;
	AuraAIController = Cast<AAuraAIController>(NewController);

	if (AuraAIController && BehaviorTree)
	{
		AuraAIController->GetBlackboardComponent()->InitializeBlackboard(*BehaviorTree->BlackboardAsset);
		AuraAIController->RunBehaviorTree(BehaviorTree);
		UE_LOG(LogTemp, Log, TEXT("AuraEnermy [%s]: Behavior Tree started (%s)."), *GetName(), *BehaviorTree->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AuraEnermy [%s]: AI not started. AuraAIController=%d BehaviorTree=%d. Set AI Controller + Behavior Tree on enemy blueprint."),
			*GetName(), AuraAIController != nullptr, BehaviorTree != nullptr);
	}
}

AAuraEnermy::AAuraEnermy()
{
	// Ensure enemies can always be found by cursor visibility traces.
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	GetMesh()->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
	Weapon->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
	GetMesh()->SetRenderCustomDepth(false);
	Weapon->SetRenderCustomDepth(false);

	AbilitySystemComponent = CreateDefaultSubobject<UAuraAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	// 能力系统组件需要在网络上进行复制，以便服务器和客户端都能访问它
	AbilitySystemComponent->SetIsReplicated(true);
	AttributeSet = CreateDefaultSubobject<UAuraAttributeSet>(TEXT("AttributeSet"));
	// 设置复制模式为Minimal，这意味着只有必要的数据会被复制，以减少网络带宽的使用
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);


	//把UI组件放在角色身上，UI组件需要从角色身上获取属性变化事件，所以它需要知道角色的ASC组件和AttributeSet
	HealthBar = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBar"));
	HealthBar->SetupAttachment(GetRootComponent());

	//GetHitMontage = CreateDefaultSubobject<UAnimMontage>(TEXT("GetHitMontage"));

	BindAttributeChangeDelegate();

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;


}

void AAuraEnermy::HighlightEnermy()
{
	UE_LOG(LogTemp, Log, TEXT("Highlight Enermy"));
	bIsHighlighted = true;

	GetMesh()->SetRenderCustomDepth(true);
	GetMesh()->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
	Weapon->SetRenderCustomDepth(true);
	Weapon->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
}

void AAuraEnermy::UnHighlightEnermy()
{
	UE_LOG(LogTemp, Log, TEXT("UnHighlight Enermy"));
	bIsHighlighted = false;
	GetMesh()->SetRenderCustomDepth(false);
	Weapon->SetRenderCustomDepth(false);
}

void AAuraEnermy::Die()
{
	SetLifeSpan(DeathLifeSpan);
	Super::Die();
	
}

void AAuraEnermy::BindAttributeChangeDelegate()
{
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UAuraAttributeSet::GetHealthAttribute()).AddUObject(this, &AAuraEnermy::OnHealthChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UAuraAttributeSet::GetMaxHealthAttribute()).AddUObject(this, &AAuraEnermy::OnMaxHealthChanged);
}

void AAuraEnermy::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	OnHealthChangedDelegate.Broadcast(Data.NewValue);
	
}

void AAuraEnermy::OnMaxHealthChanged(const FOnAttributeChangeData& Data)
{
	OnMaxHealthChangedDelegate.Broadcast(Data.NewValue);
}









void AAuraEnermy::BeginPlay()
{
	Super::BeginPlay();
	InitAbilityActorInfo();

	GetAbilitySystemComponent()->RegisterGameplayTagEvent(FGameplayTag::RequestGameplayTag(FName("Effects.HitReact")), EGameplayTagEventType::NewOrRemoved).AddUObject(this, &AAuraEnermy::GetHitReact);
	UAuraAbilitySystemLibrary::AddCharacterAbilities(this, GetAbilitySystemComponent());
}


void AAuraEnermy::GetHitReact(const FGameplayTag CallbackTag, int32 NewCount)
{
	/*bIsReactingToHit = NewCount > 0;
	GetCharacterMovement()->MaxWalkSpeed = bIsReactingToHit ? 0.f : WalkSpeed;*/

	//激活受击能力
	GetAbilitySystemComponent()->TryActivateAbilitiesByTag(FGameplayTagContainer(FGameplayTag::RequestGameplayTag(FName("Effects.HitReact"))));
}



void AAuraEnermy::InitAbilityActorInfo()
{
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	Cast<UAuraAbilitySystemComponent>(GetAbilitySystemComponent())->AbilityActorInfoSet();
	InitAttributeByCharacterInfo();
	InitializeDefaultAttributes();
}
