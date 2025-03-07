// Copyright Kyle Murray


#include "AbilitySystem/Abilities/Passive/RPGLifeSiphon.h"

FString URPGLifeSiphon::GetDescription(int32 Level)
{
	const int32 ScaledSiphonDamage = SiphonDamage.GetValueAtLevel(Level);
	return FString::Printf(TEXT(
		// Name
		"<Title>Life Siphon</>\n"

		// Level
		"<Default>Level: %d</>\n"

		// Description
		"<Default>Drains </>"
		"<FireDamage>%d </><Default>health from an enemy</>"), Level, 5);
}

FString URPGLifeSiphon::GetNextLevelDescription(int32 Level)
{
	const int32 ScaledSiphonDamage = SiphonDamage.GetValueAtLevel(Level);
	const int32 LastLevelScaledSiphonDamage = SiphonDamage.GetValueAtLevel(Level - 1);

	return FString::Printf(TEXT(
		// Name
		"<Title>Life Siphon</>\n"

		// Level
		"<Default>Level: %d</>\n"

		// Description
		"<Default>Drains </>"
		"<FireDamage>%d </><Default>health from an enemy</>"), Level, 5);
}
