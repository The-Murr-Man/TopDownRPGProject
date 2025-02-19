// Copyright Kyle Murray

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/RPGBeamSpell.h"
#include "RPGElectrocute.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API URPGElectrocute : public URPGBeamSpell
{
	GENERATED_BODY()

public:
	virtual FString GetDescription(int32 Level) override;
	virtual FString GetNextLevelDescription(int32 Level) override;
};
