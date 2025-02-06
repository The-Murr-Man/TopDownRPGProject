// Copyright Kyle Murray


#include "AbilitySystem/Abilities/RPGDamageGameplayAbility.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Interaction/CombatInterface.h"

/// <summary>
/// 
/// </summary>
/// <param name="TargetActor"></param>
void URPGDamageGameplayAbility::CauseDamage(AActor* TargetActor)
{
	FGameplayEffectSpecHandle DamageSpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass, 1);

	for (TTuple<FGameplayTag, FScalableFloat> Pair : DamageTypes)
	{
		// Gets Damage at the abilities level from curve;
		const float ScaledDamage = Pair.Value.GetValueAtLevel(GetAbilityLevel());
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(DamageSpecHandle, Pair.Key, ScaledDamage);
	}

	GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToTarget(*DamageSpecHandle.Data.Get(), UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor));
}

/// <summary>
/// 
/// </summary>
/// <param name="TaggedMontages"></param>
/// <returns></returns>
FTaggedMontage URPGDamageGameplayAbility::GetRandomTaggedMontageFromArray(const TArray<FTaggedMontage>& TaggedMontages)
{
	if (!TaggedMontages.IsEmpty())
	{
		const int32 Selection = FMath::RandRange(0, TaggedMontages.Num() - 1);
		return TaggedMontages[Selection];
	}

	return FTaggedMontage();
}

/// <summary>
/// 
/// </summary>
/// <param name="InLevel"></param>
/// <param name="DamageType"></param>
/// <returns></returns>
float URPGDamageGameplayAbility::GetDamageByDamageType(float InLevel, const FGameplayTag& DamageType)
{
	checkf(DamageTypes.Contains(DamageType), TEXT("Gameplay Ability [%s] Does not Contain DamageType [%s] "), *GetNameSafe(this), *DamageType.ToString());

	return DamageTypes[DamageType].GetValueAtLevel(InLevel);
}
