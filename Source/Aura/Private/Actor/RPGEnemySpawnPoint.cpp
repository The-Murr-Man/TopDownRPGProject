// Copyright Kyle Murray


#include "Actor/RPGEnemySpawnPoint.h"
#include "Character/RPGEnemy.h"

void ARPGEnemySpawnPoint::SpawnEnemy()
{
	FActorSpawnParameters SpawnParams;

	// Spawn even if stuck
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ARPGEnemy* Enemy = GetWorld()->SpawnActorDeferred<ARPGEnemy>(EnemyClass, GetActorTransform());
	Enemy->SetEnemyLevel(EnemyLevel);
	Enemy->SetCharacterClass(CharacterClass);
	Enemy->FinishSpawning(GetActorTransform());

	Enemy->SpawnDefaultController();
}
