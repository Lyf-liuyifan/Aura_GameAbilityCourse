// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/AuraProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/AudioComponent.h"
#include "../Aura.h"
#include "Character/AuraCharacterBase.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AAuraProjectile::AAuraProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	//创建球体组件，设置球体组件各参数
	Sphere = CreateDefaultSubobject<USphereComponent>("Sphere");
	SetRootComponent(Sphere);
	Sphere->SetCollisionObjectType(ECC_Projectile);
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement");
	ProjectileMovement->InitialSpeed = 550.f;
	ProjectileMovement->MaxSpeed = 550.f;
	ProjectileMovement->ProjectileGravityScale = 0.f;

	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AAuraProjectile::OnSphereOverlap);
	FlySoundComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("FlySoundComponent"));
	FlySoundComponent->SetupAttachment(Sphere);
	FlySoundComponent->bAutoActivate = false;
	
}

void AAuraProjectile::Destroyed()
{
	if(!bIsHit && !HasAuthority())
	{
		if (ImpactSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation(), FRotator::ZeroRotator);
		}
		if (ImpactEffect)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ImpactEffect, GetActorLocation());
		}
		FlySoundComponent->Stop();
	}
	Super::Destroyed();
}



void AAuraProjectile::EnterHeldState()
{
	// 标记为握持，后续逻辑不当作已发射
	bHeld = true;

	if (ProjectileMovement)
	{
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->Deactivate();
	}

	// 握持时不参与碰撞，避免和角色自己 Overlap
	Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (FlySoundComponent && FlySoundComponent->IsPlaying())
	{
		FlySoundComponent->Stop();
	}
}

void AAuraProjectile::LaunchToward(const FVector& TargetLocation)
{
	if (!HasAuthority()) return;

	// 先脱离皮兜 Socket，保留当前世界坐标
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	bHeld = false;

	const FVector LaunchLocation = GetActorLocation();
	FRotator LaunchRotation = (TargetLocation - LaunchLocation).Rotation();
	// 与 CastFireBolt 一致：暂不使用抛物线，水平直线飞向目标
	LaunchRotation.Pitch = 0.f;
	SetActorRotation(LaunchRotation);

	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	if (ProjectileMovement)
	{
		ProjectileMovement->Activate(true);
		ProjectileMovement->Velocity = LaunchRotation.Vector() * ProjectileMovement->InitialSpeed;
	}

	if (FlySound)
	{
		FlySoundComponent->SetSound(FlySound);
		FlySoundComponent->Play();
	}
}

// Called when the game starts or when spawned
void AAuraProjectile::BeginPlay()
{
	Super::BeginPlay();

	// 握持状态下不播飞行音效，等 LaunchToward 再播
	if (bHeld) return;

	FlySoundComponent->SetSound(FlySound);
	FlySoundComponent->Play();
}

void AAuraProjectile::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OhterComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Log, TEXT("Projectile Overlap Other Actor"));
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation(), FRotator::ZeroRotator);
	}
	if (ImpactEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ImpactEffect, GetActorLocation());
	}
	if (HasAuthority())
	{
		FlySoundComponent->Stop();

		AAuraCharacterBase* TargetCharacter = Cast<AAuraCharacterBase>(OtherActor);
		if (TargetCharacter)
		{
			if (UAbilitySystemComponent* TargetASC = TargetCharacter->GetAbilitySystemComponent())
			{
				if (DamageHandle.IsValid() && DamageHandle.Data.IsValid())
				{
					TargetASC->ApplyGameplayEffectSpecToTarget(*DamageHandle.Data.Get(), TargetASC);
				}
			}
		}

		Destroy();
	}
	else
	{
		bIsHit = true;
	}




}





