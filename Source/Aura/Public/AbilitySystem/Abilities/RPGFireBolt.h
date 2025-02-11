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

	UFUNCTION(BlueprintCallable)
	void SpawnProjectiles(const FVector& ProjectileTargetLocation ,const FGameplayTag& SocketTag, bool bOverridePitch, float PitchOverride,AActor* HomingTarget);

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Firebolt")
	float ProjectileSpread = 45;

	UPROPERTY(EditDefaultsOnly, Category = "Firebolt")
	float HomingAcceleration = 3000;

	UPROPERTY(EditDefaultsOnly, Category = "Firebolt")
	bool bLaunchHomingProjectile = true;
};
