// Copyright Kyle Murray


#include "AbilitySystem/Abilities/RPGPassiveAbility.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

/// <summary>
/// Activate Given Ability
/// </summary>
/// <param name="Handle"></param>
/// <param name="ActorInfo"></param>
/// <param name="ActivationInfo"></param>
/// <param name="TriggerEventData"></param>
void URPGPassiveAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (URPGAbilitySystemComponent* RPGASC = Cast<URPGAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo())))
	{
		RPGASC->DeactivatePassiveAbilityDelegate.AddUObject(this, &URPGPassiveAbility::ReceiveDeativate);
	}
}

void URPGPassiveAbility::ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass)
{
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);

	if (!TargetASC) return;

	checkf(GameplayEffectClass, TEXT("GameplayEffectClass is uninitialized, please set GameplayEffectClass"));

	// Makes Effect Context
	FGameplayEffectContextHandle EffectContextHandle = TargetASC->MakeEffectContext();
	EffectContextHandle.AddSourceObject(this);

	// Makes Outgoing Spec
	FGameplayEffectSpecHandle EffectSpecHandle = TargetASC->MakeOutgoingSpec(GameplayEffectClass, 1, EffectContextHandle);

	// Applies Gameplay Effect Spec to Self
	FActiveGameplayEffectHandle ActiveEffectHandle = TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data);
	const bool bIsInfinite = EffectSpecHandle.Data.Get()->Def.Get()->DurationPolicy == EGameplayEffectDurationType::Infinite;

	// Checks if the effect is infinite
	if (bIsInfinite)
	{
		ActiveEffectHandles.Add(ActiveEffectHandle, TargetASC);
	}

}

void URPGPassiveAbility::RemoveEffectFromTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass)
{
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);

	if (!TargetASC) return;

	checkf(GameplayEffectClass, TEXT("GameplayEffectClass is uninitialized, please set GameplayEffectClass"));

	TArray<FActiveGameplayEffectHandle> HandlesToRemove;

	for (auto HandlePair : ActiveEffectHandles)
	{
		if (TargetASC == HandlePair.Value)
		{
			TargetASC->RemoveActiveGameplayEffect(HandlePair.Key, 1);
			HandlesToRemove.Add(HandlePair.Key);
		}
	}

	for (FActiveGameplayEffectHandle& Handle : HandlesToRemove)
	{
		ActiveEffectHandles.FindAndRemoveChecked(Handle);
	}
}

/// <summary>
/// Deactivates Ability
/// </summary>
/// <param name="AbilityTag"></param>
void URPGPassiveAbility::ReceiveDeativate(const FGameplayTag& AbilityTag)
{
	if (AbilityTags.HasTagExact(AbilityTag))
	{
		EndAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,true,true);
	}
}

