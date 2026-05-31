// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AuraCharacter.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Player/AuraPlayerState.h"
#include "Player/AuraPlayerController.h"
#include "UI/HUD/AuraHUD.h"
#include "GameFramework/CharacterMovementComponent.h"

AAuraCharacter::AAuraCharacter()
{
	//设置角色的旋转方式，使其朝向移动方向
	GetCharacterMovement()->bOrientRotationToMovement = true;

	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;


}

void AAuraCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	//Init ability actor info for the server, the client will do this in OnRep_PlayerState
	InitAbilityActorInfo();
	//AddCharacterAbilities();
	UE_LOG(LogTemp, Log, TEXT("PossessedBy========================="));
}

void AAuraCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	InitAbilityActorInfo();
	UE_LOG(LogTemp, Log, TEXT("OnRep_Playerstate Run"));
}


//初始化ASC组件和AttributeSet，并将它们传递给HUD
void AAuraCharacter::InitAbilityActorInfo()
{
	//获取PlayerState并初始化ASC组件和AttributeSet
	AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
	if (!AuraPlayerState)
	{
		return;
	}
	//ASC组件和AttributeSet都在PlayerState里，所以要先获取PlayerState
	AuraPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(AuraPlayerState, this);
	AbilitySystemComponent = Cast<UAuraAbilitySystemComponent>(AuraPlayerState->GetAbilitySystemComponent());
	AttributeSet = AuraPlayerState->GetAttributeSet();

	if (AbilitySystemComponent)
	{
		Cast<UAuraAbilitySystemComponent>(AuraPlayerState->GetAbilitySystemComponent())->AbilityActorInfoSet();
	}

	//将ASC组件和AttributeSet传递给HUD
	if (AAuraPlayerController* AuraPlayerController = Cast<AAuraPlayerController>(GetController()))
	{
		if(AAuraHUD* AuraHUD = Cast<AAuraHUD>(AuraPlayerController->GetHUD()))
		{
			AuraHUD->InitOverlay(AuraPlayerController, AuraPlayerState, AbilitySystemComponent, AttributeSet);
			AuraHUD->InitMenuWidget(AuraPlayerController, AuraPlayerState, AbilitySystemComponent, AttributeSet);
		}
	}
	//初始化Primary Attributes
	//InitializePrimaryAttributes();
	InitializeDefaultAbilities();
	AddCharacterAbilities();
}

int32 AAuraCharacter::GetPlayerLevel()
{
	if (AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>())
	{
		return AuraPlayerState->GetPlayerLevel();
	}
	return 0;
}
