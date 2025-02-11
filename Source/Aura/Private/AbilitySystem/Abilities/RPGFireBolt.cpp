// Copyright Kyle Murray


#include "AbilitySystem/Abilities/RPGFireBolt.h"
#include "RPGGameplayTags.h"
#include "Interaction/CombatInterface.h"
#include "Interaction/EnemyInterface.h"
#include "AbilitySystem/RPGAbilitySystemLibrary.h"
#include "Actor/RPGProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"

FString URPGFireBolt::GetDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);

	if (Level == 1)
	{
		return FString::Printf(TEXT(
			// Name
			"<Title>FIRE BOLT</>\n"

			// Level
			"<Default>Level: %d</>\n"

			// Mana Cost
			"<Small>Mana Cost: </><ManaCost>%.1f</>\n"

			// Cooldown
			"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"

			// Description
			"<Default>Launches a bolt of fire, exploding on impact and dealing:</>"
			"<FireDamage> %d Fire</>"
			"<Default> damage with a chance of </>"
			"<FireDamage>Burning</>"), Level,ManaCost,Cooldown, ScaledDamage);
	}

	else
	{
		return FString::Printf(TEXT(
			// Name
			"<Title>FIRE BOLT</>\n"

			// Level
			"<Default>Level: %d</>\n"

			// Mana Cost
			"<Small>Mana Cost: </><ManaCost>%.1f</>\n"

			// Cooldown
			"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"

			// Description
			"<Default>Launches %d bolts of fire, exploding on impact and dealing:</>"

			// Damage
			"<FireDamage> %d Fire</>"
			"<Default> damage with a chance of </>"
			"<FireDamage>Burning</>"), Level,ManaCost,Cooldown, FMath::Min(Level, MaxNumProjectiles), ScaledDamage);
	}
}

FString URPGFireBolt::GetNextLevelDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const int32 LastLevelScaledDamage = Damage.GetValueAtLevel(Level-1);

	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float LastLevelManaCost = FMath::Abs(GetManaCost(Level-1));

	const float Cooldown = GetCooldown(Level);
	const float LastLevelCooldown = GetCooldown(Level-1);
	return FString::Printf(TEXT(
		// Name
		"<Title>FIRE BOLT</>\n"     

		// Level
		"<Default>Level: %d > %d</>\n"

		// Mana Cost
		"<Small>Mana Cost: </><ManaCost>%.1f > %.1f</>\n"

		// Cooldown
		"<Small>Cooldown: </><Cooldown>%.1f > %.1f</>\n\n"

		// Description
		"<Default>Launches %d bolts of fire, exploding on impact and dealing:</>"

		// Damage
		"<FireDamage> %d > %d Fire</>"
		"<Default> damage with a chance of </>"
		"<FireDamage>Burning</>"), 
		
		//Values
		Level - 1, Level,
		LastLevelManaCost, ManaCost,
		LastLevelCooldown,Cooldown, 
		FMath::Min(Level, MaxNumProjectiles), 
		LastLevelScaledDamage, ScaledDamage);
}

void URPGFireBolt::SpawnProjectiles(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag, bool bOverridePitch, float PitchOverride, AActor* HomingTarget)
{
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();

	if (!bIsServer) return;

	ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetAvatarActorFromActorInfo());

	if (!CombatInterface) return;

	const FVector SocketLocation = CombatInterface->Execute_GetCombatSocketLocation(GetAvatarActorFromActorInfo(), SocketTag);
	FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation();

	if (bOverridePitch) Rotation.Pitch = PitchOverride;

	const FVector Forward = Rotation.Vector();

	const int32 NumProjectiles = FMath::Min(MaxNumProjectiles, GetAbilityLevel());
	TArray<FRotator> Rotations = URPGAbilitySystemLibrary::EvenlySpacedRotators(Forward, FVector::UpVector, ProjectileSpread, NumProjectiles);

	for (FRotator& Rot : Rotations)
	{
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(SocketLocation);

		SpawnTransform.SetRotation(Rot.Quaternion());

		AActor* OwningActor = GetOwningActorFromActorInfo();

		ARPGProjectile* Projectile = GetWorld()->SpawnActorDeferred<ARPGProjectile>(ProjectileClass, SpawnTransform, OwningActor, Cast<APawn>(OwningActor), ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		// Damage Effect Params
		Projectile->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();

		if (!HomingTarget) return;
		if (HomingTarget->Implements<UCombatInterface>())
		{
			Projectile->ProjectileMovement->HomingTargetComponent = HomingTarget->GetRootComponent();
		}

		else
		{
			Projectile->HomingTargetSceneComponent = NewObject<USceneComponent>(USceneComponent::StaticClass());
			Projectile->HomingTargetSceneComponent->SetWorldLocation(ProjectileTargetLocation);
			Projectile->ProjectileMovement->HomingTargetComponent = Projectile->HomingTargetSceneComponent;
		}


		Projectile->ProjectileMovement->HomingAccelerationMagnitude = HomingAcceleration;
		Projectile->ProjectileMovement->bIsHomingProjectile = bLaunchHomingProjectile;
		Projectile->FinishSpawning(SpawnTransform);
	}
}
