// Copyright Kyle Murray


#include "AbilitySystem/Abilities/RPGGameplayAbility.h"
#include "AbilitySystem/RPGAttributeSet.h"

/// <summary>
/// Returns the Description of the Gameplay Ability
/// </summary>
/// <param name="Level"></param>
/// <returns></returns>
FString URPGGameplayAbility::GetDescription(int32 Level)
{
	return FString::Printf(TEXT("<Default> %s, </> <Level> %d</>"), L"Default Ability Name - LoremIpsum LoremIpsum LoremIpsum LoremIpsum LoremIpsum", Level);
}

/// <summary>
/// Returns the Description of the Gameplay Abilities next level
/// </summary>
/// <param name="Level"></param>
/// <returns></returns>
FString URPGGameplayAbility::GetNextLevelDescription(int32 Level)
{
	return FString::Printf(TEXT("<Default> Next Level: </> <Level> %d</> \n <Default> Is Much Cooler </>"), Level);
}

/// <summary>
/// Returns the Description of a locked Ability
/// </summary>
/// <param name="Level"></param>
/// <returns></returns>
FString URPGGameplayAbility::GetLockedDescription(int32 Level)
{
	return FString::Printf(TEXT("<Default> Ability Locked Until:  </> <Level> %d</>"), Level);
}

/// <summary>
/// Returns the ManaCost of a ability
/// </summary>
/// <param name="InLevel"></param>
/// <returns></returns>
float URPGGameplayAbility::GetManaCost(float InLevel)
{
	float ManaCost = 0;
	const UGameplayEffect* CostEffect = GetCostGameplayEffect();

	if (!CostEffect) return 0;

	for (FGameplayModifierInfo Mod : CostEffect->Modifiers)
	{
		if (Mod.Attribute == URPGAttributeSet::GetManaAttribute())
		{
			Mod.ModifierMagnitude.GetStaticMagnitudeIfPossible(InLevel, ManaCost);
			break;
		}
	}

	return ManaCost;
}

/// <summary>
/// Returns the Cooldown of a ability
/// </summary>
/// <param name="InLevel"></param>
/// <returns></returns>
float URPGGameplayAbility::GetCooldown(float InLevel)
{
	float Cooldown = 0;
	const UGameplayEffect* CooldownEffect = GetCooldownGameplayEffect();

	if (!CooldownEffect) return 0;

	CooldownEffect->DurationMagnitude.GetStaticMagnitudeIfPossible(InLevel, Cooldown);

	return Cooldown;
}
