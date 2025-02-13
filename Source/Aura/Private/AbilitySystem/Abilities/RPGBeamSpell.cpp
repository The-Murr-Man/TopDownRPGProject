// Copyright Kyle Murray


#include "AbilitySystem/Abilities/RPGBeamSpell.h"
#include "RPGGameplayTags.h"
#include "Interaction/CombatInterface.h"
#include "Interaction/EnemyInterface.h"
#include "AbilitySystem/RPGAbilitySystemLibrary.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"

FString URPGBeamSpell::GetDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);

	if (Level == 1)
	{
		return FString::Printf(TEXT(
			// Name
			"<Title>Electrocute </>\n"

			// Level
			"<Default>Level: %d</>\n"

			// Mana Cost
			"<Small>Mana Cost: </><ManaCost>%.1f</>\n"

			// Cooldown
			"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"

			// Description
			"<Default>Launches a bolt of fire, exploding on impact and dealing:</>"
			"<LightningDamage> %d Lightning</>"
			"<Default> damage with a chance of </>"
			"<LightningDamage>Stunning</>"), Level, ManaCost, Cooldown, ScaledDamage);
	}

	else
	{
		return FString::Printf(TEXT(
			// Name
			"<Title>Electrocute</>\n"

			// Level
			"<Default>Level: %d</>\n"

			// Mana Cost
			"<Small>Mana Cost: </><ManaCost>%.1f</>\n"

			// Cooldown
			"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"

			// Description
			"<Default>Launches  bolts of fire, exploding on impact and dealing:</>"

			// Damage
			"<LightningDamage> %d Lightning</>"
			"<Default> damage with a chance of </>"
			"<LightningDamage>Stunning</>"), Level, ManaCost, Cooldown, ScaledDamage);
	}
}

FString URPGBeamSpell::GetNextLevelDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const int32 LastLevelScaledDamage = Damage.GetValueAtLevel(Level - 1);

	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float LastLevelManaCost = FMath::Abs(GetManaCost(Level - 1));

	const float Cooldown = GetCooldown(Level);
	const float LastLevelCooldown = GetCooldown(Level - 1);

	return FString::Printf(TEXT(
		// Name
		"<Title>Electrocute</>\n"

		// Level
		"<Default>Level: %d > %d</>\n"

		// Mana Cost
		"<Small>Mana Cost: </><ManaCost>%.1f > %.1f</>\n"

		// Cooldown
		"<Small>Cooldown: </><Cooldown>%.1f > %.1f</>\n\n"

		// Description
		"<Default>Launches bolts of fire, exploding on impact and dealing:</>"

		// Damage
		"<LightningDamage> %d > %d Lightning</>"
		"<Default> damage with a chance of </>"
		"<LightningDamage>Stunning</>"),

		//Values
		Level - 1, Level,
		LastLevelManaCost, ManaCost,
		LastLevelCooldown, Cooldown,
		LastLevelScaledDamage, ScaledDamage);
}

/// <summary>
/// 
/// </summary>
/// <param name="HitResult"></param>
void URPGBeamSpell::StoreMouseDataInfo(const FHitResult& HitResult)
{
	if (HitResult.bBlockingHit)
	{
		MouseHitLocation = HitResult.ImpactPoint;
		MouseHitActor = HitResult.GetActor();
	}

	else
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}
}

/// <summary>
/// 
/// </summary>
void URPGBeamSpell::StoreOwnerVariables()
{
	if (CurrentActorInfo)
	{
		OwnerPlayerController = CurrentActorInfo->PlayerController.Get(); // Get called because its a weak object pointer
		OwnerCharacter = Cast<ACharacter>(CurrentActorInfo->AvatarActor);
	}
}

/// <summary>
/// 
/// </summary>
/// <param name="BeamTargetLocation"></param>
void URPGBeamSpell::TraceFirstTarget(const FVector& BeamTargetLocation)
{
	check(OwnerCharacter);

	if (OwnerCharacter->Implements<UCombatInterface>())
	{
		if (USkeletalMeshComponent* Weapon = ICombatInterface::Execute_GetWeaponMesh(OwnerCharacter))
		{
			TArray<AActor*> ActorsToIgnore;
			ActorsToIgnore.Add(OwnerCharacter);

			FHitResult HitResult;
			const FVector SocketLocation = Weapon->GetSocketLocation(WeaponSocketName);

			UKismetSystemLibrary::SphereTraceSingle(OwnerCharacter, SocketLocation, BeamTargetLocation, 10, TraceTypeQuery1, false, ActorsToIgnore, EDrawDebugTrace::None, HitResult, true);

			// Hit Result hit something
			if (HitResult.bBlockingHit)
			{
				MouseHitLocation = HitResult.ImpactPoint;
				MouseHitActor = HitResult.GetActor();
			}
		}
	}
}

/// <summary>
/// 
/// </summary>
/// <param name="OutAdditionalTargets"></param>
void URPGBeamSpell::StoreAdditionalTargets(TArray<AActor*>& OutAdditionalTargets)
{
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(GetAvatarActorFromActorInfo());
	ActorsToIgnore.Add(MouseHitActor);

	TArray<AActor*> OverlappingActors;

	URPGAbilitySystemLibrary::GetLivePlayersWithinRadius(GetAvatarActorFromActorInfo(), OverlappingActors, ActorsToIgnore, BeamChainRadius, MouseHitActor->GetActorLocation());

	// Remove players from overlapped actors
	for (int32 i = 0; i < OverlappingActors.Num(); i++)
	{
		if (!OverlappingActors[i]->ActorHasTag(FName("Enemy")))
		{
			OverlappingActors.RemoveAt(i);
		}
	}

	int32 NumAdditionalTargets = FMath::Min(GetAbilityLevel() - 1, MaxNumShockTargets); 

	URPGAbilitySystemLibrary::GetClosestTargets(NumAdditionalTargets, OverlappingActors, OutAdditionalTargets, MouseHitActor->GetActorLocation());
}
