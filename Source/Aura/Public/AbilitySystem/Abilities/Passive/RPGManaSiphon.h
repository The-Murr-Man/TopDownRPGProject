// Copyright Kyle Murray

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/RPGPassiveAbility.h"
#include "RPGManaSiphon.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API URPGManaSiphon : public URPGPassiveAbility
{
	GENERATED_BODY()
	
public:
	virtual FString GetDescription(int32 Level) override;
	virtual FString GetNextLevelDescription(int32 Level) override;

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Mana Siphon")
	FScalableFloat ManaSiphon;
};
