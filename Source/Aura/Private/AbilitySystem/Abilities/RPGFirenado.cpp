// Copyright Kyle Murray


#include "AbilitySystem/Abilities/RPGFirenado.h"

FString URPGFirenado::GetDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);

	if (Level == 1)
	{
		return FString::Printf(TEXT(
			// Name
			"<Title>Firenado</>\n"

			// Level
			"<Default>Level: %d</>\n"

			// Mana Cost
			"<Small>Mana Cost: </><ManaCost>%.1f</>\n"

			// Cooldown
			"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"

			// Description
			"<Default>Spawns a Tornado of fire dealing:</>"
			"<FireDamage> %d Fire</>"
			"<Default> damage within a radius </>"), Level, ManaCost, Cooldown, Level, ScaledDamage);
	}

	else
	{
		return FString::Printf(TEXT(
			// Name
			"<Title>Firenado</>\n"

			// Level
			"<Default>Level: %d</>\n"

			// Mana Cost
			"<Small>Mana Cost: </><ManaCost>%.1f</>\n"

			// Cooldown
			"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"

			// Description
			"<Default>Spawns a Tornado of fire dealing:</>"

			// Damage
			"<FireDamage> %d Fire</>"
			"<Default> damage within a radius </>"), Level, ManaCost, Cooldown, Level, ScaledDamage);
	}
}

FString URPGFirenado::GetNextLevelDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const int32 LastLevelScaledDamage = Damage.GetValueAtLevel(Level - 1);

	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float LastLevelManaCost = FMath::Abs(GetManaCost(Level - 1));

	const float Cooldown = GetCooldown(Level);
	const float LastLevelCooldown = GetCooldown(Level - 1);

	return FString::Printf(TEXT(
		// Name
		"<Title>Firenado</>\n"

		// Level
		"<Default>Level: %d > %d</>\n"

		// Mana Cost
		"<Small>Mana Cost: </><ManaCost>%.1f > %.1f</>\n"

		// Cooldown
		"<Small>Cooldown: </><Cooldown>%.1f > %.1f</>\n\n"

		// Description
		"<Default>Spawns a Tornado of fire dealing:</>"

		// Damage
		"<FireDamage> %d > %d Fire</>"
		"<Default> damage within a radius </>"),

		//Values
		Level - 1, Level,
		LastLevelManaCost, ManaCost,
		LastLevelCooldown, Cooldown,
		Level,
		LastLevelScaledDamage, ScaledDamage);
}
