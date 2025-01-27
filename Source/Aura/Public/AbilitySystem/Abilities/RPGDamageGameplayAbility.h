// Copyright Kyle Murray

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/RPGGameplayAbility.h"
#include "RPGDamageGameplayAbility.generated.h"

// Forward Declarations
class UGameplayEffect;

/**
 * 
 */
UCLASS()
class AURA_API URPGDamageGameplayAbility : public URPGGameplayAbility
{
	GENERATED_BODY()
	
public:

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	TMap<FGameplayTag, FScalableFloat> DamageTypes;
};
