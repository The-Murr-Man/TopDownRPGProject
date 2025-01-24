// Copyright Kyle Murray

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "RPGAbilitySystemGlobals.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API URPGAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()
	

	virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;
};
