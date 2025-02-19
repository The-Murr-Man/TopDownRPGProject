// Copyright Kyle Murray


#include "AbilitySystem/Abilities/RPGElectrocute.h"

FString URPGElectrocute::GetDescription(int32 Level)
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
			"<Default>Fires a beam of continuous Lightning causing:</>"
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
			"<Default>Fires a beam of continuous Lightning chaining to %d additional targets causing:</>"

			// Damage
			"<LightningDamage> %d Lightning</>"
			"<Default> damage with a chance of </>"
			"<LightningDamage>Stunning</>"), Level, ManaCost, Cooldown, Level -1, ScaledDamage);
	}
}

FString URPGElectrocute::GetNextLevelDescription(int32 Level)
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
		"<Default>Fires a beam of continuous Lightning chaining to %d additional targets causing:</>"

		// Damage
		"<LightningDamage> %d > %d Lightning</>"
		"<Default> damage with a chance of </>"
		"<LightningDamage>Stunning</>"),

		//Values
		Level - 1, Level,
		LastLevelManaCost, ManaCost,
		LastLevelCooldown, Cooldown,
		Level - 1,
		LastLevelScaledDamage, ScaledDamage);
}