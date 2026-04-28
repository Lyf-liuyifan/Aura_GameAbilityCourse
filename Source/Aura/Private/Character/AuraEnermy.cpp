// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AuraEnermy.h"
#include "../Aura.h"
#include "Components/CapsuleComponent.h"

AAuraEnermy::AAuraEnermy()
{
	// Ensure enemies can always be found by cursor visibility traces.
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	GetMesh()->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
	Weapon->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
	GetMesh()->SetRenderCustomDepth(false);
	Weapon->SetRenderCustomDepth(false);
}

void AAuraEnermy::HighlightEnermy()
{
	UE_LOG(LogTemp, Log, TEXT("Highlight Enermy"));
	bIsHighlighted = true;

	//取出骨骼网格体高亮
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
