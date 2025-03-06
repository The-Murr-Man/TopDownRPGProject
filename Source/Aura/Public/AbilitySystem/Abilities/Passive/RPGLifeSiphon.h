// Copyright Kyle Murray

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/RPGPassiveAbility.h"
#include "RPGLifeSiphon.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API URPGLifeSiphon : public URPGPassiveAbility
{
	GENERATED_BODY()
	
public:
	virtual FString GetDescription(int32 Level) override;
	virtual FString GetNextLevelDescription(int32 Level) override;

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Life Siphon")
	FScalableFloat SiphonDamage;
};
