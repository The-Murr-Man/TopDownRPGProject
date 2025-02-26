// Copyright Kyle Murray

#pragma once

#include "CoreMinimal.h"
#include "Engine/TargetPoint.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "RPGEnemySpawnPoint.generated.h"

class ARPGEnemy;

/**
 * 
 */
UCLASS()
class AURA_API ARPGEnemySpawnPoint : public ATargetPoint
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable)
	void SpawnEnemy();

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "EnemyClass")
	TSubclassOf<ARPGEnemy> EnemyClass;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "EnemyClass")
	int32 EnemyLevel = 1;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "EnemyClass")
	ECharacterClass CharacterClass = ECharacterClass::Warrior;

protected:

private:
};
