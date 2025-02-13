// Copyright Kyle Murray


#include "AbilitySystem/Abilities/RPGPassiveAbility.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

void URPGPassiveAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (URPGAbilitySystemComponent* RPGASC = Cast<URPGAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo())))
	{
		RPGASC->DeactivatePassiveAbilityDelegate.AddUObject(this, &URPGPassiveAbility::ReceiveDeativate);
	}
}

void URPGPassiveAbility::ReceiveDeativate(const FGameplayTag& AbilityTag)
{
	if (AbilityTags.HasTagExact(AbilityTag))
	{
		EndAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,true,true);
	}
}
