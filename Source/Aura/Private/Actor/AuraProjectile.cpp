// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/AuraProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/PlayerState.h"
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

bool AAuraProjectile::ShouldIgnoreOverlapTarget(AActor* OtherActor) const
{
	if (!OtherActor)
	{
		return true;
	}

	// 忽略发射者 Pawn（石头/火球的 Instigator）
	if (OtherActor == GetInstigator())
	{
		return true;
	}

	// 忽略 Owner（敌人 ASC 在自身时 Owner=自己；玩家可能是 PlayerState，仍需判）
	if (OtherActor == GetOwner())
	{
		return true;
	}

	// Owner 是 PlayerState / Controller 时，再忽略其控制的 Pawn（火球打自己）
	if (const AController* OwnerController = Cast<AController>(GetOwner()))
	{
		if (OtherActor == OwnerController->GetPawn())
		{
			return true;
		}
	}
	else if (const APlayerState* OwnerPS = Cast<APlayerState>(GetOwner()))
	{
		if (OtherActor == OwnerPS->GetPawn())
		{
			return true;
		}
	}

	return false;
}

void AAuraProjectile::ApplyMoveIgnoreForShooter()
{
	if (!Sphere)
	{
		return;
	}

	// 物理/扫描层面也忽略发射者，减少刚启用碰撞时立刻 Overlap 自己
	if (AActor* InstigatorActor = GetInstigator())
	{
		Sphere->IgnoreActorWhenMoving(InstigatorActor, true);
	}
	if (AActor* OwnerActor = GetOwner())
	{
		Sphere->IgnoreActorWhenMoving(OwnerActor, true);
		if (const APlayerState* OwnerPS = Cast<APlayerState>(OwnerActor))
		{
			if (APawn* OwnerPawn = OwnerPS->GetPawn())
			{
				Sphere->IgnoreActorWhenMoving(OwnerPawn, true);
			}
		}
	}
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

	// 启用碰撞前先忽略发射者，避免出膛瞬间打中自己
	ApplyMoveIgnoreForShooter();
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

	// 火球等非握持投射物：生成后立刻忽略发射者
	if (!bHeld)
	{
		ApplyMoveIgnoreForShooter();
	}

	// 握持状态下不播飞行音效，等 LaunchToward 再播
	if (bHeld) return;

	FlySoundComponent->SetSound(FlySound);
	FlySoundComponent->Play();
}

void AAuraProjectile::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OhterComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 忽略发射者自己，避免石头/火球自伤触发 HitReact 导致停步
	if (ShouldIgnoreOverlapTarget(OtherActor))
	{
		return;
	}

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
