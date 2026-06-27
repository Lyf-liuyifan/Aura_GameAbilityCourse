#pragma once

#include "CoreMinimal.h"
#include "Character/AuraCharacterBase.h"
#include "AuraLabTargetDummy.generated.h"

UCLASS()
class AURA_API AAuraLabTargetDummy : public AAuraCharacterBase
{
	GENERATED_BODY()

public:
	AAuraLabTargetDummy();

	virtual int32 GetPlayerLevel() override { return Level; }

protected:
	virtual void BeginPlay() override;
	virtual void InitAbilityActorInfo() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lab")
	int32 Level = 1;
};
