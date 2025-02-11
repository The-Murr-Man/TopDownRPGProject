// Copyright Kyle Murray

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/RPGGameplayAbility.h"
#include "RPGAbilityTypes.h"
#include "RPGDamageGameplayAbility.generated.h"

// Forward Declarations
class UGameplayEffect;
class ICombatInterface;
/**
 * 
 */
UCLASS()
class AURA_API URPGDamageGameplayAbility : public URPGGameplayAbility
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable)
	void CauseDamage(AActor* TargetActor);

	UFUNCTION(BlueprintPure)
	FTaggedMontage GetRandomTaggedMontageFromArray(const TArray<FTaggedMontage>& TaggedMontages);

	UFUNCTION(BlueprintPure)
	FDamageEffectParams MakeDamageEffectParamsFromClassDefaults(AActor* TargetActor = nullptr) const;

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	FGameplayTag DamageType;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	FScalableFloat Damage;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float DeathImpulseMagnitude = 1000;

	UPROPERTY(EditDefaultsOnly, Category = "Damage|Knockback")
	float KnockbackForceMagnitude = 1000;

	UPROPERTY(EditDefaultsOnly, Category = "Damage|Knockback")
	float KnockbackChance = 0;

	/// <summary>
	/// TODO: Make Scalable Floats and Create Curve Tables
	/// </summary>
	UPROPERTY(EditDefaultsOnly, Category = "Damage|Debuff")
	float DebuffChance = 20;

	UPROPERTY(EditDefaultsOnly, Category = "Damage|Debuff")
	float DebuffDamage = 5;

	UPROPERTY(EditDefaultsOnly, Category = "Damage|Debuff")
	float DebuffFrequency = 1;

	UPROPERTY(EditDefaultsOnly, Category = "Damage|Debuff")
	float DebuffDuration = 5;
};
