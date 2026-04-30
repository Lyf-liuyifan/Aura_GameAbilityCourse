// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/AuraEffectActor.h"
#include "Components/SphereComponent.h"
#include "Player/AuraPlayerState.h"
#include "AbilitySystem/AuraAttributeSet.h"

// Sets default values
AAuraEffectActor::AAuraEffectActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	//创建球体组件
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	SetRootComponent(StaticMeshComponent);
	SphereComponent->SetupAttachment(GetRootComponent());


}

void AAuraEffectActor::OnSphereOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//实现自己的的重叠事件逻辑，首先拿到重叠角色的状态组件，判断是否有效
	//如果有效，调用状态组件的接口，改变数值
	if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(OtherActor))\
	{
		//拿到状态组件
		const UAuraAttributeSet* AuraAttributeSet = Cast<UAuraAttributeSet>(ASCInterface->GetAbilitySystemComponent()->GetAttributeSet(UAuraAttributeSet::StaticClass()));

		UAuraAttributeSet* MutableAuraAttributeSet = const_cast<UAuraAttributeSet*>(AuraAttributeSet);
		MutableAuraAttributeSet->SetHealth(AuraAttributeSet->GetHealth() + 25.f);
		UE_LOG(LogTemp, Log, TEXT("MutableAuraAttributeSet Health is %f"), MutableAuraAttributeSet->GetHealth());
		UE_LOG(LogTemp, Log, TEXT("AuraAttributeSet Health is %f"), AuraAttributeSet->GetHealth());
		Destroy();
	}


}

void AAuraEffectActor::OnSphereOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{


}

// Called when the game starts or when spawned
void AAuraEffectActor::BeginPlay()
{
	Super::BeginPlay();
	//绑定了球体组件的重叠事件
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AAuraEffectActor::OnSphereOverlapBegin);
	SphereComponent->OnComponentEndOverlap.AddDynamic(this, &AAuraEffectActor::OnSphereOverlapEnd);



}


