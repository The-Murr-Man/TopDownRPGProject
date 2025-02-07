// Copyright Kyle Murray


#include "AbilitySystem/Abilities/RPGFireBolt.h"
#include "RPGGameplayTags.h"

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
			"<FireDamage>Burning</>"), Level,ManaCost,Cooldown, FMath::Min(Level, NumProjectiles), ScaledDamage);
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
		FMath::Min(Level, NumProjectiles), 
		LastLevelScaledDamage, ScaledDamage);
}