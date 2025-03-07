// Copyright Kyle Murray

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/RPGDamageGameplayAbility.h"
#include "RPGFirenado.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API URPGFirenado : public URPGDamageGameplayAbility
{
	GENERATED_BODY()

public:

	virtual FString GetDescription(int32 Level) override;
	virtual FString GetNextLevelDescription(int32 Level) override;
};
