// Copyright Kyle Murray


#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "RPGGameplayTags.h"
#include "AbilitySystem/Abilities/RPGGameplayAbility.h"
#include "RPGLogChannels.h"
#include "Interaction/PlayerInterface.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/RPGAbilitySystemLibrary.h"
#include "AbilitySystem/Data/AbilityInfo.h"

/// <summary>
/// 
/// </summary>
void URPGAbilitySystemComponent::AbilityActorInfoSet()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &URPGAbilitySystemComponent::ClientEffectApplied);
}

/// <summary>
/// 
/// </summary>
/// <param name="StartupAbilities"></param>
void URPGAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities)
{
	for (TSubclassOf<UGameplayAbility> AbilityClass : StartupAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);

		if (const URPGGameplayAbility* RPGAbility = Cast<URPGGameplayAbility>(AbilitySpec.Ability))
		{
			AbilitySpec.DynamicAbilityTags.AddTag(RPGAbility->StartupInputTag);
			AbilitySpec.DynamicAbilityTags.AddTag(FRPGGameplayTags::Get().Abilities_Status_Equipped);
			GiveAbility(AbilitySpec);
		}
	}

	bStartupAbilitiesGiven = true;
	//
	AbilitiesGivenDelegate.Broadcast();
}

/// <summary>
/// 
/// </summary>
/// <param name="StartupPassiveAbilities"></param>
void URPGAbilitySystemComponent::AddCharacterPassiveAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupPassiveAbilities)
{
	for (TSubclassOf<UGameplayAbility> AbilityClass : StartupPassiveAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);

		GiveAbilityAndActivateOnce(AbilitySpec);
	}
}

/// <summary>
/// 
/// </summary>
/// <param name="InputTag"></param>
void URPGAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	// Looping through all activatible Abilities
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		// Checks if InputTag is in DynamicAbilityTags
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			AbilitySpecInputPressed(AbilitySpec);
			// Checks if Ability Spec is already active
			if (!AbilitySpec.IsActive())
			{
				// Activate the ability
				TryActivateAbility(AbilitySpec.Handle);
			}
		}
	}
}

/// <summary>
/// 
/// </summary>
/// <param name="InputTag"></param>
void URPGAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	// Looping through all activatible Abilities
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		// Checks if InputTag is in DynamicAbilityTags
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			AbilitySpecInputReleased(AbilitySpec);
		}
	}
}

/// <summary>
/// 
/// </summary>
/// <param name="Delegate"></param>
void URPGAbilitySystemComponent::ForEachAbility(const FForEachAbility Delegate)
{
	FScopedAbilityListLock ActiveScopeLock(*this);

	for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (!Delegate.ExecuteIfBound(AbilitySpec))
		{
			UE_LOG(LogRPG,Error, TEXT("Failed to execute delegat in %hs"), __FUNCTION__) // %hs logs the name of a function
		}
	}
}

/// <summary>
/// 
/// </summary>
/// <param name="Level"></param>
void URPGAbilitySystemComponent::UpdateAbilityStatuses(int32 Level)
{
	UAbilityInfo* AbilityInfo = URPGAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());

	for (const FRPGAbilityInfo& Info : AbilityInfo->AbilityInformation)
	{
		// Continue if Ability Tag isnt valid
		if (!Info.AbilityTag.IsValid()) continue;

		// Continue if Ability Level is less than Level Requirment
		if (Level < Info.LevelRequirement) continue;

		// Ability Doesnt exist in our Activatable Abilities
		if (GetSpecFromAbilityTag(Info.AbilityTag) == nullptr)
		{
			// Create Ability spec for new ability at level 1(Will load later)
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(Info.Ability, 1);

			// Changing status of ability to eligible since we meets its level requirement
			AbilitySpec.DynamicAbilityTags.AddTag(FRPGGameplayTags::Get().Abilities_Status_Eligible);

			GiveAbility(AbilitySpec);

			// Forces ability spec to replicate Now
			MarkAbilitySpecDirty(AbilitySpec);

			ClientUpdateAbilityStatus(Info.AbilityTag, FRPGGameplayTags::Get().Abilities_Status_Eligible);
		}
	}
}

/// <summary>
/// Returns the Ability tag from a Ability Spec
/// </summary>
/// <param name="AbilitySpec"></param>
/// <returns></returns>
FGameplayTag URPGAbilitySystemComponent::GetAbilityTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	if (AbilitySpec.Ability)
	{
		for (FGameplayTag AbilityTag : AbilitySpec.Ability->AbilityTags)
		{
			if (AbilityTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Abilities"))))
			{
				return AbilityTag;
			}
		}
	}
	
	return FGameplayTag();
}

/// <summary>
/// Returns the Input tag from a Ability Spec
/// </summary>
/// <param name="AbilitySpec"></param>
/// <returns></returns>
FGameplayTag URPGAbilitySystemComponent::GetInputTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	for (FGameplayTag InputTag : AbilitySpec.DynamicAbilityTags)
	{
		if (InputTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("InputTag"))))
		{
			return InputTag;
		}
	}

	return FGameplayTag();
}

/// <summary>
/// Returns the Status tag from a Ability Spec
/// </summary>
/// <param name="AbilitySpec"></param>
/// <returns></returns>
FGameplayTag URPGAbilitySystemComponent::GetStatusFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	for (FGameplayTag StatusTag : AbilitySpec.DynamicAbilityTags)
	{
		if (StatusTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Abilities.Status"))))
		{
			return StatusTag;
		}
	}
	return FGameplayTag();
}

/// <summary>
/// Returns the Ability spec from a specified Ability Tag
/// </summary>
/// <param name="AbilityTag"></param>
/// <returns></returns>
FGameplayAbilitySpec* URPGAbilitySystemComponent::GetSpecFromAbilityTag(const FGameplayTag& AbilityTag)
{
	FScopedAbilityListLock ActiveScopeLock(*this);

	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		for (FGameplayTag Tag : AbilitySpec.Ability->AbilityTags)
		{
			if (Tag.MatchesTagExact(AbilityTag))
			{
				return &AbilitySpec;
			}
		}
	}
	return nullptr;
}

/// <summary>
/// 
/// </summary>
/// <param name="AttributeTag"></param>
void URPGAbilitySystemComponent::UpgradeAttribute(const FGameplayTag& AttributeTag)
{
	if (GetAvatarActor()->Implements<UPlayerInterface>())
	{
		if (IPlayerInterface::Execute_GetAttributePoints(GetAvatarActor()) > 0)
		{
			ServerUpgradeAttribute(AttributeTag);
		}
	}
}

/// <summary>
/// RPC For Upgrading Attribute Points
/// </summary>
/// <param name="AttributeTag"></param>
void URPGAbilitySystemComponent::ServerUpgradeAttribute_Implementation(const FGameplayTag& AttributeTag)
{
	FGameplayEventData Payload;
	Payload.EventTag = AttributeTag;
	Payload.EventMagnitude = 1;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetAvatarActor(), AttributeTag, Payload);

	if (GetAvatarActor()->Implements<UPlayerInterface>())
	{
		IPlayerInterface::Execute_AddToAttributePoints(GetAvatarActor(), -1);
	}
}

/// <summary>
/// 
/// </summary>
void URPGAbilitySystemComponent::OnRep_ActivateAbilities()
{
	Super::OnRep_ActivateAbilities();

	if (!bStartupAbilitiesGiven)
	{
		bStartupAbilitiesGiven = true;
		AbilitiesGivenDelegate.Broadcast();
	}
}

/// <summary>
/// Client RPC For Broadcasting the AbilityStatusChangedDelegate
/// </summary>
/// <param name="AbilityTag"></param>
/// <param name="StatusTag"></param>
void URPGAbilitySystemComponent::ClientUpdateAbilityStatus_Implementation(const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag)
{
	AbilityStatusChangedDelegate.Broadcast(AbilityTag, StatusTag);
}

/// <summary>
/// 
/// </summary>
/// <param name="AbilitySystemComponent"></param>
/// <param name="EffectSpec"></param>
/// <param name="ActiveEffectHandle"></param>
void URPGAbilitySystemComponent::ClientEffectApplied_Implementation(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle)
{
	FGameplayTagContainer TagContainer;
	EffectSpec.GetAllAssetTags(TagContainer);

	EffectAssetTags.Broadcast(TagContainer);
}
