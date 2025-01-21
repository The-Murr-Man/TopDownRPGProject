// Copyright Kyle Murray

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/RPGGameplayAbility.h"
#include "RPGProjectileSpell.generated.h"

class ARPGProjectile;
/**
 * 
 */
UCLASS()
class AURA_API URPGProjectileSpell : public URPGGameplayAbility
{
	GENERATED_BODY()
	
public:


protected:

	// Overiding ActivateAbility() from GameplayAbility.h
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData);

	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	TSubclassOf<ARPGProjectile> ProjectileClass;
};
