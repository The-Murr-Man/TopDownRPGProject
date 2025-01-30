// Copyright Kyle Murray

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/RPGGameplayAbility.h"
#include "RPGSummonAbility.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API URPGSummonAbility : public URPGGameplayAbility
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable)
	TArray<FVector> GetSpawnLocations();

	UFUNCTION(BlueprintPure, Category = "Summoning")
	TSubclassOf<APawn> GetRandomMinionClass();

	UPROPERTY(EditDefaultsOnly, Category = "Summoning")
	int32 NumMinions = 5;

	UPROPERTY(EditDefaultsOnly, Category = "Summoning")
	TArray<TSubclassOf<APawn>> MinionClasses;

	UPROPERTY(EditDefaultsOnly, Category = "Summoning")
	float MinSpawnDistance= 100;

	UPROPERTY(EditDefaultsOnly, Category = "Summoning")
	float MaxSpawnDistance= 300;

	UPROPERTY(EditDefaultsOnly, Category = "Summoning")
	float SpawnSpread = 90;
};
