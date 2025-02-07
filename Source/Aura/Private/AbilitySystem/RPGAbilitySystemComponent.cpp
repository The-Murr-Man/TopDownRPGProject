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
			UE_LOG(LogRPG,Error, TEXT("Failed to execute delegate in %hs"), __FUNCTION__) // %hs logs the name of a function
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

			ClientUpdateAbilityStatus(Info.AbilityTag, FRPGGameplayTags::Get().Abilities_Status_Eligible, 1);
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

FGameplayTag URPGAbilitySystemComponent::GetStatusFromAbilityTag(const FGameplayTag& AbilityTag)
{
	if (const FGameplayAbilitySpec* Spec = GetSpecFromAbilityTag(AbilityTag))
	{
		GetStatusFromSpec(*Spec);
	}
	return FGameplayTag();
}

FGameplayTag URPGAbilitySystemComponent::GetInputTagFromAbilityTag(const FGameplayTag& AbilityTag)
{
	if (const FGameplayAbilitySpec* Spec = GetSpecFromAbilityTag(AbilityTag))
	{
		GetInputTagFromSpec(*Spec);
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

bool URPGAbilitySystemComponent::GetDescriptionsByAbilityTag(const FGameplayTag& AbilityTag, FString& OutDescription, FString& OutNextLevelDescription)
{
	if (const FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		if (URPGGameplayAbility* RPGAbility = Cast<URPGGameplayAbility>(AbilitySpec->Ability))
		{
			OutDescription = RPGAbility->GetDescription(AbilitySpec->Level);
			OutNextLevelDescription = RPGAbility->GetNextLevelDescription(AbilitySpec->Level + 1);
			return true;
		}
	}

	UAbilityInfo* AbilityInfo = URPGAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());

	if (!AbilityTag.IsValid() || AbilityTag.MatchesTagExact(FRPGGameplayTags::Get().Abilities_None))
		OutDescription = FString();
	
	else
		OutDescription = URPGGameplayAbility::GetLockedDescription(AbilityInfo->FindAbilityInfoForTag(AbilityTag).LevelRequirement);
	
	OutNextLevelDescription = FString();

	return false;
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
/// Removes Input Tag from Slot
/// </summary>
/// <param name="AbilitySpec"></param>
void URPGAbilitySystemComponent::ClearSlot(FGameplayAbilitySpec* AbilitySpec)
{
	const FGameplayTag& Slot = GetInputTagFromSpec(*AbilitySpec);

	AbilitySpec->DynamicAbilityTags.RemoveTag(Slot);
	MarkAbilitySpecDirty(*AbilitySpec);
}

/// <summary>
/// Iterates through ActivatableAbilities and Clears Slot if the Ability has one
/// </summary>
/// <param name="Slot"></param>
void URPGAbilitySystemComponent::ClearSlotAbilities(const FGameplayTag& Slot)
{
	FScopedAbilityListLock ActiveScopeLock(*this);

	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilityHasSlot(&AbilitySpec, Slot))
		{
			ClearSlot(&AbilitySpec);
		}
	}
}

/// <summary>
/// Iterates through Given Specs Dynamic Ability Tags and returns if an Ability Has a Slot
/// </summary>
/// <param name="AbilitySpec"></param>
/// <param name="Slot"></param>
/// <returns></returns>
bool URPGAbilitySystemComponent::AbilityHasSlot(FGameplayAbilitySpec* AbilitySpec, const FGameplayTag& Slot)
{
	for (FGameplayTag Tag : AbilitySpec->DynamicAbilityTags)
	{
		if (Tag.MatchesTagExact(Slot)) return true;
	}

	return false;
}

/// <summary>
/// Server RPC For Equipping a Ability
/// </summary>
/// <param name="AbilityTag"></param>
/// <param name="Slot"></param>
void URPGAbilitySystemComponent::ServerEquipAbility_Implementation(const FGameplayTag& AbilityTag, const FGameplayTag& Slot)
{
	if (FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		const FRPGGameplayTags& GameplayTags = FRPGGameplayTags::Get();
		const FGameplayTag& PreviousSlot = GetInputTagFromSpec(*AbilitySpec);
		const FGameplayTag& Status = GetStatusFromSpec(*AbilitySpec);

		// Only Valid if Status is Equipped or Unlocked
		const bool bStatusValid = Status == GameplayTags.Abilities_Status_Equipped || Status == GameplayTags.Abilities_Status_Unlocked;

		if (bStatusValid)
		{
			// Remove this InputTag (Slot) from any Ability that has it
			ClearSlotAbilities(Slot);
			
			// Clear this ability's slot, just in case its a different slot
			ClearSlot(AbilitySpec);

			// Now, assign this ability to this slot
			AbilitySpec->DynamicAbilityTags.AddTag(Slot);

			if (Status.MatchesTagExact(GameplayTags.Abilities_Status_Unlocked))
			{
				// Removes Unlocked tag and Adds Equipped Tag
				AbilitySpec->DynamicAbilityTags.RemoveTag(GameplayTags.Abilities_Status_Unlocked);
				AbilitySpec->DynamicAbilityTags.AddTag(GameplayTags.Abilities_Status_Equipped);
			}

			MarkAbilitySpecDirty(*AbilitySpec);
		}

		ClientEquipAbility(AbilityTag, GameplayTags.Abilities_Status_Equipped, Slot, PreviousSlot);
	}
}

/// <summary>
/// Server RPC For Spending a Spell point
/// </summary>
/// <param name="AbilityTag"></param>
void URPGAbilitySystemComponent::ServerSpendSpellPoint_Implementation(const FGameplayTag& AbilityTag)
{
	if (FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		// Decrement Spell Point
		if (GetAvatarActor()->Implements<UPlayerInterface>())
		{
			IPlayerInterface::Execute_AddToSpellPoints(GetAvatarActor(), -1);
		}

		FRPGGameplayTags GameplayTags = FRPGGameplayTags::Get();
		FGameplayTag Status = GetStatusFromSpec(*AbilitySpec);

		if (Status.MatchesTagExact(GameplayTags.Abilities_Status_Eligible))
		{
			// Removes the Eligible Tag
			AbilitySpec->DynamicAbilityTags.RemoveTag(GameplayTags.Abilities_Status_Eligible);

			// Add the Unlocked Tag
			AbilitySpec->DynamicAbilityTags.AddTag(GameplayTags.Abilities_Status_Unlocked);

			Status = GameplayTags.Abilities_Status_Unlocked;
		}

		else if (Status.MatchesTagExact(GameplayTags.Abilities_Status_Equipped) || Status.MatchesTagExact(GameplayTags.Abilities_Status_Unlocked))
		{
			AbilitySpec->Level++;
		}

		ClientUpdateAbilityStatus(AbilityTag, Status, AbilitySpec->Level);
		MarkAbilitySpecDirty(*AbilitySpec);
	}
}

/// <summary>
/// Server RPC For Upgrading Attribute Points
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
/// Client RPC For Broadcasting AbilityEquippedDelegate
/// </summary>
/// <param name="AbilityTag"></param>
/// <param name="Status"></param>
/// <param name="Slot"></param>
/// <param name="PreviousSlot"></param>
void URPGAbilitySystemComponent::ClientEquipAbility_Implementation(const FGameplayTag& AbilityTag, const FGameplayTag& Status, const FGameplayTag& Slot, const FGameplayTag& PreviousSlot)
{
	AbilityEquippedDelegate.Broadcast(AbilityTag, Status, Slot, PreviousSlot);
}

/// <summary>
/// Client RPC For Broadcasting the AbilityStatusChangedDelegate
/// </summary>
/// <param name="AbilityTag"></param>
/// <param name="StatusTag"></param>
void URPGAbilitySystemComponent::ClientUpdateAbilityStatus_Implementation(const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, int32 AbilityLevel)
{
	AbilityStatusChangedDelegate.Broadcast(AbilityTag, StatusTag, AbilityLevel);
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
