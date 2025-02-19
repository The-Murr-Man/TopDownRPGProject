// Copyright Kyle Murray


#include "AbilitySystem/Abilities/RPGFireBlast.h"
#include "AbilitySystem/RPGAbilitySystemLibrary.h"
#include "Actor/RPGFireBall.h"
#include "Kismet/GameplayStatics.h"

FString URPGFireBlast::GetDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);

	return FString::Printf(TEXT(
		// Name
		"<Title>Fire Blast</>\n"

		// Level
		"<Default>Level: %d</>\n"

		// Mana Cost
		"<Small>Mana Cost: </><ManaCost>%.1f</>\n"

		// Cooldown
		"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"

		// Description
		"<Default>Launches %d balls of fire in all directions, :</>"
		"<Default>each coming back and dealing:</>"
		"<FireDamage> %d Fire</>"
		"<Default> damage with a chance of </>"
		"<FireDamage>Buring</>"), Level, ManaCost, Cooldown, NumFireBalls, ScaledDamage);
}

FString URPGFireBlast::GetNextLevelDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const int32 LastLevelScaledDamage = Damage.GetValueAtLevel(Level - 1);

	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float LastLevelManaCost = FMath::Abs(GetManaCost(Level - 1));

	const float Cooldown = GetCooldown(Level);
	const float LastLevelCooldown = GetCooldown(Level - 1);

	return FString::Printf(TEXT(
		// Name
		"<Title>Fire Blast</>\n"

		// Level
		"<Default>Level: %d > %d</>\n"

		// Mana Cost
		"<Small>Mana Cost: </><ManaCost>%.1f > %.1f</>\n"

		// Cooldown
		"<Small>Cooldown: </><Cooldown>%.1f > %.1f</>\n\n"

		// Description
		"<Default>Launches %d balls of fire in all directions, :</>"
		"<Default>each coming back and dealing:</>"

		// Damage
		"<FireDamage> %d > %d Fire</>"
		"<Default> damage with a chance of </>"
		"<FireDamage>Buring</>"),

		//Values
		Level - 1, Level,
		LastLevelManaCost, ManaCost,
		LastLevelCooldown, Cooldown,
		NumFireBalls,
		LastLevelScaledDamage, ScaledDamage);
}

TArray<ARPGFireBall*> URPGFireBlast::SpawnFireBalls()
{
	TArray<ARPGFireBall*> FireBalls;

	const FVector Forward = GetAvatarActorFromActorInfo()->GetActorForwardVector();
	const FVector Location = GetAvatarActorFromActorInfo()->GetActorLocation();

	TArray<FRotator> Rotators = URPGAbilitySystemLibrary::EvenlySpacedRotators(Forward, FVector::UpVector, 360, NumFireBalls);

	for (const FRotator& Rot : Rotators)
	{
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(Location);
		SpawnTransform.SetRotation(Rot.Quaternion());

		// Spawn
		ARPGFireBall* FireBall = GetWorld()->SpawnActorDeferred<ARPGFireBall>(FireBallClass, SpawnTransform, GetOwningActorFromActorInfo(),CurrentActorInfo->PlayerController->GetPawn(), 
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		FireBall->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();
		FireBall->SetOwner(GetAvatarActorFromActorInfo());

		// Damage Params for the explosion
		FireBall->ExplosionDamageParams = MakeDamageEffectParamsFromClassDefaults();
		FireBall->SetOwner(GetAvatarActorFromActorInfo());

		FireBalls.Add(FireBall);

		FireBall->FinishSpawning(SpawnTransform);
	}

	return FireBalls;
}
