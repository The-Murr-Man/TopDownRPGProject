// Copyright Kyle Murray


#include "AbilitySystem/Abilities/Passive/RPGHaloOfProtection.h"

FString URPGHaloOfProtection::GetDescription(int32 Level)
{
	//const int32 ScaledDamageReduction = GetDamageReductionValueFromCurve(Level);
	return FString::Printf(TEXT(
		// Name
		"<Title>Halo Of Protection</>\n"

		// Level
		"<Default>Level: %d</>\n"

		// Description
		"<Default>Reduces damage taken by</>\n"
		"<LightningDamage>20%%</>"), Level);
}

FString URPGHaloOfProtection::GetNextLevelDescription(int32 Level)
{
	//const int32 ScaledDamageReduction = GetDamageReductionValueFromCurve(Level);
	//const int32 LastLevelDamageReduction = GetDamageReductionValueFromCurve(Level - 1);

	return FString::Printf(TEXT(
		// Name
		"<Title>Halo Of Protection</>\n"

		// Level
		"<Default>Level: %d</>\n"

		// Description
		"<Default>Reduces damage taken by</>\n"
		"<LightningDamage>20%%</>"), Level);
}
