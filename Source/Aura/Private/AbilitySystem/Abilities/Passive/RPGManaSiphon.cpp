// Copyright Kyle Murray


#include "AbilitySystem/Abilities/Passive/RPGManaSiphon.h"

FString URPGManaSiphon::GetDescription(int32 Level)
{
	const int32 ScaledSiphonedMana = ManaSiphon.GetValueAtLevel(Level);
	return FString::Printf(TEXT(
		// Name
		"<Title>Mana Siphon</>\n"

		// Level
		"<Default>Level: %d</>\n"

		// Description
		"<Default>Drains </>"
		"<ManaCost>%d </><Default>mana from an enemy</>"), Level, 5);
}

FString URPGManaSiphon::GetNextLevelDescription(int32 Level)
{
	const int32 ScaledSiphonedMana = ManaSiphon.GetValueAtLevel(Level);
	const int32 LastLevelScaledSiphonedMana = ManaSiphon.GetValueAtLevel(Level - 1);

	return FString::Printf(TEXT(
		// Name
		"<Title>Mana Siphon</>\n"

		// Level
		"<Default>Level: %d </>\n"

		// Description
		"<Default>Drains </>"
		"<ManaCost>%d </><Default>mana from an enemy</>"), Level, 5);
}