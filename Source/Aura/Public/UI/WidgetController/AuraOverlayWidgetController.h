// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "UI/Widget/AuraUserWidget.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "AuraOverlayWidgetController.generated.h"

/**
 * 
 */



USTRUCT(BlueprintType)
struct FUIWidgetRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag MessageTag = FGameplayTag();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Message = FText();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UAuraUserWidget> WidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UTexture2D* Icon = nullptr;
};

//添加几个代理，多播代理
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChangedSignature, float, Health);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxHealthChangedSignature, float, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnManaChangedSignature, float, Mana);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxManaChangedSignature, float, MaxMana);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMessageWidgetRowSignature, FUIWidgetRow, WidgetRow);



UCLASS(BlueprintType, Blueprintable)
class AURA_API UAuraOverlayWidgetController : public UAuraWidgetController
{
	GENERATED_BODY()

public:
		virtual void BroadcastInitialValues() override;
		virtual void BindCallbacksToDependencies() override;

		UPROPERTY(BlueprintAssignable, Category="GAS|Attribute")
		FOnHealthChangedSignature OnHealthChanged;

		UPROPERTY(BlueprintAssignable, Category = "GAS|Attribute")
		FOnMaxHealthChangedSignature OnMaxHealthChanged;

		UPROPERTY(BlueprintAssignable, Category = "GAS|Attribute")
		FOnManaChangedSignature OnManaChanged;

		UPROPERTY(BlueprintAssignable, Category = "GAS|Attribute")
		FOnMaxManaChangedSignature OnMaxManaChanged;

		UPROPERTY(BlueprintAssignable, Category = "GAS|Attribute")
		FMessageWidgetRowSignature MessageWidgetRowDelegate;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WidgetData")
	TObjectPtr<UDataTable> MessageDataTable;

	void HealthChanged(const FOnAttributeChangeData& Data);
	void MaxHealthChanged(const FOnAttributeChangeData& Data);
	void ManaChanged(const FOnAttributeChangeData& Data);
	void MaxManaChanged(const FOnAttributeChangeData& Data);

	template<typename T>
	T* GetDataTableRowByTag(UDataTable* DataTable, const FGameplayTag& Tag) const
	{
		if (!DataTable || !Tag.IsValid())
		{
			return nullptr;
		}
		return DataTable->FindRow<T>(Tag.GetTagName(), TEXT(""));
	}
};
