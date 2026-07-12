// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"
#include "AuraProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UNiagaraSystem;

UCLASS()
class AURA_API AAuraProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAuraProjectile();

	void Destroyed() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OhterComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:	
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	TObjectPtr<USphereComponent> Sphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UNiagaraSystem> ImpactEffect;

	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundBase> ImpactSound;

	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundBase> FlySound;

	TObjectPtr<UAudioComponent> FlySoundComponent;

	


	bool bIsHit = false;

	/** 是否处于拉弓握持状态（Attach 在皮兜上，尚未发射） */
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bHeld = false;

	UPROPERTY(BlueprintReadWrite, meta = (ExposeOnSpawn = true))
	FGameplayEffectSpecHandle DamageHandle;

	/** 是否应忽略该 Overlap 目标（发射者 / Owner / PlayerState 对应 Pawn） */
	bool ShouldIgnoreOverlapTarget(AActor* OtherActor) const;

	/** 在启用碰撞前，把发射者加入 MoveIgnore，减少出膛瞬间自撞 */
	void ApplyMoveIgnoreForShooter();

	/** 进入握持：关闭 Movement 和碰撞，停止飞行音效 */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void EnterHeldState();

	/** 松手发射：Detach 后启用 Movement，直线飞向目标 */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void LaunchToward(const FVector& TargetLocation);
};
