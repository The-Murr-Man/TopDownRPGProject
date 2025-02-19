// Copyright Kyle Murray


#include "AbilitySystem/Abilities/RPGArcaneShards.h"

FString URPGArcaneShards::GetDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);

	if (Level == 1)
	{
		return FString::Printf(TEXT(
			// Name
			"<Title>Arcane Shards</>\n"

			// Level
			"<Default>Level: %d</>\n"

			// Mana Cost
			"<Small>Mana Cost: </><ManaCost>%.1f</>\n"

			// Cooldown
			"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"

			// Description
			"<Default>Spawns a cluster of Arcane energy dealing:</>"
			"<ArcaneDamage> %d Arcane</>"
			"<Default> damage within a radius </>"
			"<ArcaneDamage>TBD</>"), Level, ManaCost, Cooldown, Level, ScaledDamage);
	}

	else
	{
		return FString::Printf(TEXT(
			// Name
			"<Title>Arcane Shards</>\n"

			// Level
			"<Default>Level: %d</>\n"

			// Mana Cost
			"<Small>Mana Cost: </><ManaCost>%.1f</>\n"

			// Cooldown
			"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"

			// Description
			"<Default>Spawns %d clusters of Arcane energy dealing:</>"

			// Damage
			"<ArcaneDamage> %d Arcane</>"
			"<Default> damage within a radius </>"
			"<LightningDamage>Stunning</>"), Level, ManaCost, Cooldown, Level, ScaledDamage);
	}
}

FString URPGArcaneShards::GetNextLevelDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const int32 LastLevelScaledDamage = Damage.GetValueAtLevel(Level - 1);

	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float LastLevelManaCost = FMath::Abs(GetManaCost(Level - 1));

	const float Cooldown = GetCooldown(Level);
	const float LastLevelCooldown = GetCooldown(Level - 1);

	return FString::Printf(TEXT(
		// Name
		"<Title>Arcane Shards</>\n"

		// Level
		"<Default>Level: %d > %d</>\n"

		// Mana Cost
		"<Small>Mana Cost: </><ManaCost>%.1f > %.1f</>\n"

		// Cooldown
		"<Small>Cooldown: </><Cooldown>%.1f > %.1f</>\n\n"

		// Description
		"<Default>Spawns %d clusters of Arcane energy dealing:</>"

		// Damage
		"<ArcaneDamage> %d > %d Arcane</>"
		"<Default> damage within a radius </>"
		"<ArcaneDamage>TBD</>"),

		//Values
		Level - 1, Level,
		LastLevelManaCost, ManaCost,
		LastLevelCooldown, Cooldown,
		Level,
		LastLevelScaledDamage, ScaledDamage);
}
