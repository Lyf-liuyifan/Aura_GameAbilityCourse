// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "Components/SplineComponent.h"
#include "AuraPlayerController.generated.h"

/**
 * 
 */
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class IEnermyInterface;
class UAuraInputConfig;
class UAuraAbilitySystemComponent;
class USplineComponent;


UCLASS()
class AURA_API AAuraPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AAuraPlayerController();
	virtual void PlayerTick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	virtual void SetupInputComponent() override;


private:
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> AuraContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;


	IEnermyInterface* LastActor;
	IEnermyInterface* FocusedActor;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UAuraInputConfig> InputConfig;

	UPROPERTY()
	TObjectPtr<UAuraAbilitySystemComponent> AuraAbilitySystemComponent;

	UAuraAbilitySystemComponent* GetASC();


	void AbilityInputPressed(FGameplayTag InputTag);
	void AbilityInputReleased(FGameplayTag InputTag);
	void AbilityInputHeld(FGameplayTag InputTag);

	void Move(const FInputActionValue& Value);

	void CursorTrace();


	/* Character Move By Cursor Clicked */
	FVector CachedDestination = FVector::ZeroVector;
	float FollowTime = 0.f;
	float ShortThreshold = 0.5f;
	bool bAutoRunning = false;
	UPROPERTY(EditDefaultsOnly)
	float AutoRunAcceptanceRadius = 50.f;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USplineComponent> Spline;
	bool bIsTargeting = false;
};
