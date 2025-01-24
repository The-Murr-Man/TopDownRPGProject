// Copyright Kyle Murray


#include "AbilitySystem/RPGAbilitySystemGlobals.h"
#include "RPGAbilityTypes.h"

FGameplayEffectContext* URPGAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FRPGGameplayEffectContext();
}
