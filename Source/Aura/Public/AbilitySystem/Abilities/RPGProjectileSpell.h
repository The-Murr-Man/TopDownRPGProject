// Copyright Kyle Murray

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/RPGDamageGameplayAbility.h"
#include "RPGProjectileSpell.generated.h"

// Forward Declarations
class ARPGProjectile;
class UGameplayEffect;
struct FGameplayTag;
/**
 * 
 */
UCLASS()
class AURA_API URPGProjectileSpell : public URPGDamageGameplayAbility
{
	GENERATED_BODY()
	
protected:

	// Overiding ActivateAbility() from GameplayAbility.h
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData);

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void SpawnProjectile(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag, bool bOverridePitch = false, float PitchOverride = 0);

	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	TSubclassOf<ARPGProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly)
	int32 MaxNumProjectiles = 5;
};
