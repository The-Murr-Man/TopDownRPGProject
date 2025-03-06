// Copyright Kyle Murray

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/RPGPassiveAbility.h"
#include "RPGHaloOfProtection.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API URPGHaloOfProtection : public URPGPassiveAbility
{
	GENERATED_BODY()
	
public:
	virtual FString GetDescription(int32 Level) override;
	virtual FString GetNextLevelDescription(int32 Level) override;
protected:
};
