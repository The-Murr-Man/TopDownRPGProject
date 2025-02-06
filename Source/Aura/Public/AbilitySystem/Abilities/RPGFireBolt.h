// Copyright Kyle Murray

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/RPGProjectileSpell.h"
#include "RPGFireBolt.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API URPGFireBolt : public URPGProjectileSpell
{
	GENERATED_BODY()

public:
	virtual FString GetDescription(int32 Level) override;
	virtual FString GetNextLevelDescription(int32 Level) override;
};
