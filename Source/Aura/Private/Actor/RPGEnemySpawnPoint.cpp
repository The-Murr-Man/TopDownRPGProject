// Copyright Kyle Murray


#include "Actor/RPGEnemySpawnPoint.h"
#include "Character/RPGEnemy.h"

void ARPGEnemySpawnPoint::SpawnEnemy()
{
	FActorSpawnParameters SpawnParams;

	// Spawn even if stuck
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Spawn Enemy
	ARPGEnemy* Enemy = GetWorld()->SpawnActorDeferred<ARPGEnemy>(EnemyClass, GetActorTransform());

	// Sets level and class of enemy
	Enemy->SetEnemyLevel(EnemyLevel);
	Enemy->SetCharacterClass(CharacterClass);

	// Finishes Spawning
	Enemy->FinishSpawning(GetActorTransform());

	// Sets the controller
	Enemy->SpawnDefaultController();
}
