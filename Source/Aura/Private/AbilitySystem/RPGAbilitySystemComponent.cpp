// Copyright Kyle Murray


#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "RPGGameplayTags.h"
#include "AbilitySystem/Abilities/RPGGameplayAbility.h"
#include "RPGLogChannels.h"
#include "Interaction/PlayerInterface.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/RPGAbilitySystemLibrary.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "Game/LoadScreenSaveGame.h"

void URPGAbilitySystemComponent::AbilityActorInfoSet()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &URPGAbilitySystemComponent::ClientEffectApplied);
}

/// <summary>
/// Give abilities to character on startup and assign the input/status tags
/// </summary>
/// <param name="StartupAbilities"></param>
void URPGAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities)
{
	// Loops throuhg StartupAbilities
	for (TSubclassOf<UGameplayAbility> AbilityClass : StartupAbilities)
	{
		// Creates AbilitySpec from AbilityClass and Level
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);

		if (const URPGGameplayAbility* RPGAbility = Cast<URPGGameplayAbility>(AbilitySpec.Ability))
		{
			// Add Gameplay Tags for Input and Status
			AbilitySpec.DynamicAbilityTags.AddTag(RPGAbility->StartupInputTag);
			AbilitySpec.DynamicAbilityTags.AddTag(FRPGGameplayTags::Get().Abilities_Status_Equipped);

			// Give the ability
			GiveAbility(AbilitySpec);
		}
	}

	bStartupAbilitiesGiven = true;

	// Broadcast the Abilities have been given
	AbilitiesGivenDelegate.Broadcast();
}

/// <summary>
/// Give abilities to character from save data and assign the input/status tags
/// </summary>
/// <param name="SaveData"></param>
void URPGAbilitySystemComponent::AddCharacterAbilitiesFromSaveData(ULoadScreenSaveGame* SaveData)
{
	// Loops through the SaveData's SavedAbilities
	for (const FSavedAbility& Data : SaveData->SavedAbilities)
	{
		// Create GameplayAbility from given Data
		const TSubclassOf<UGameplayAbility> LoadedAbilityClass = Data.GameplayAbility;

		// Create GameplayAbilitySpec from given Data
		FGameplayAbilitySpec LoadedAbilitySpec = FGameplayAbilitySpec(LoadedAbilityClass, Data.AbilityLevel);

		// Adds Data's Status and Slot to the DynamicTags
		LoadedAbilitySpec.DynamicAbilityTags.AddTag(Data.AbilitySlot);
		LoadedAbilitySpec.DynamicAbilityTags.AddTag(Data.AbilityStatus);

		// Checks if Ability has the Offensive Tag
		if (Data.AbilityType.MatchesTagExact(FRPGGameplayTags::Get().Abilities_Type_Offensive))
		{
			GiveAbility(LoadedAbilitySpec);
		}

		// Checks if Ability has the Passive Tag
		else if (Data.AbilityType.MatchesTagExact(FRPGGameplayTags::Get().Abilities_Type_Passive))
		{
			GiveAbility(LoadedAbilitySpec);

			// Checks if the ability is equipped on load
			if (Data.AbilityStatus.MatchesTagExact(FRPGGameplayTags::Get().Abilities_Status_Equipped))
			{
				// Try to activate the ability
				TryActivateAbility(LoadedAbilitySpec.Handle);

				// Activate the passive effect
				MulticastActivatePassiveEffect(Data.AbilityTag, true);
			}
		}
	}

	bStartupAbilitiesGiven = true;
	AbilitiesGivenDelegate.Broadcast();
}

/// <summary>
/// Give passive abilities to character on startup
/// </summary>
/// <param name="StartupPassiveAbilities"></param>
void URPGAbilitySystemComponent::AddCharacterPassiveAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupPassiveAbilities)
{
	// Loops through StartupPassiveAbilities
	for (TSubclassOf<UGameplayAbility> AbilityClass : StartupPassiveAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);

		AbilitySpec.DynamicAbilityTags.AddTag(FRPGGameplayTags::Get().Abilities_Status_Equipped);

		GiveAbilityAndActivateOnce(AbilitySpec);
	}
}

/// <summary>
/// Handles what happens when Abilities input tag is pressed
/// </summary>
/// <param name="InputTag"></param>
void URPGAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	FScopedAbilityListLock ActiveScopeLock(*this);
	// Looping through all activatible Abilities
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		// Checks if InputTag is in DynamicAbilityTags
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			AbilitySpecInputPressed(AbilitySpec);

			// Checks if Ability Spec is already active
			if (AbilitySpec.IsActive())
			{
				// REQUIRED TO USE WAITINPUT PRESSED AND RELEASED IN BP
				InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, AbilitySpec.Handle, AbilitySpec.ActivationInfo.GetActivationPredictionKey());
			}
		}
	}
}

/// <summary>
///  Handles what happens when Abilities input tag is held
/// </summary>
/// <param name="InputTag"></param>
void URPGAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	FScopedAbilityListLock ActiveScopeLock(*this);
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
///  Handles what happens when Abilities input tag is released
/// </summary>
/// <param name="InputTag"></param>
void URPGAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	FScopedAbilityListLock ActiveScopeLock(*this);
	// Looping through all activatible Abilities
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		// Checks if InputTag is in DynamicAbilityTags
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag) && AbilitySpec.IsActive())
		{
			AbilitySpecInputReleased(AbilitySpec);

			// REQUIRED TO USE WAIT INPUT PRESSED AND RELEASED IN BP
			InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, AbilitySpec.Handle, AbilitySpec.ActivationInfo.GetActivationPredictionKey());
		}
	}
}

/// <summary>
/// Executes the delegate for each Ability
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
/// Updates the status for all Abilities
/// </summary>
/// <param name="Level"></param>
void URPGAbilitySystemComponent::UpdateAbilityStatuses(int32 Level)
{
	UAbilityInfo* AbilityInfo = URPGAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());

	// Loops through AbilityInformation
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
/// Calls Server Version of UpgradeAttributes
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
		if (AbilityHasSlot(AbilitySpec, Slot))
		{
			ClearSlot(&AbilitySpec);
		}
	}
}

/// <summary>
/// Returns whether an Ability has a slot
/// </summary>
/// <param name="Spec"></param>
/// <param name="Slot"></param>
/// <returns></returns>
bool URPGAbilitySystemComponent::AbilityHasSlot(FGameplayAbilitySpec& Spec, const FGameplayTag& Slot)
{
	return Spec.DynamicAbilityTags.HasTagExact(Slot);
}

/// <summary>
/// Returns whether any slot has the InputTag
/// </summary>
/// <param name="Spec"></param>
/// <returns></returns>
bool URPGAbilitySystemComponent::AbilityHasAnySlot(FGameplayAbilitySpec& Spec)
{
	return Spec.DynamicAbilityTags.HasTag(FGameplayTag::RequestGameplayTag(FName("InputTag")));
}

/// <summary>
/// Return whether an ability is passive
/// </summary>
/// <param name="Spec"></param>
/// <returns></returns>
bool URPGAbilitySystemComponent::IsPassiveAbility(const FGameplayAbilitySpec& Spec) const
{
	UAbilityInfo* AbilityInfo = URPGAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	const FGameplayTag AbilityTag = GetAbilityTagFromSpec(Spec);
	const FRPGAbilityInfo& Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
	const FGameplayTag AbilityType = Info.AbilityType;

	return AbilityType.MatchesTagExact(FRPGGameplayTags::Get().Abilities_Type_Passive);
}

/// <summary>
/// Returns whether a slot is empty
/// </summary>
/// <param name="Slot"></param>
/// <returns></returns>
bool URPGAbilitySystemComponent::SlotIsEmpty(const FGameplayTag& Slot)
{
	FScopedAbilityListLock ActiveScopeLock(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilityHasSlot(AbilitySpec, Slot))
		{
			return false;
		}
	}
	return true;
}

/// <summary>
/// Sets the given slot to given spec
/// </summary>
/// <param name="Spec"></param>
/// <param name="Slot"></param>
void URPGAbilitySystemComponent::AssignSlotToAbility(FGameplayAbilitySpec& Spec, const FGameplayTag& Slot)
{
	// Clear the slot
	ClearSlot(&Spec);

	// Add the new slot
	Spec.DynamicAbilityTags.AddTag(Slot);
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

		// Check if abilities status is valid
		if (bStatusValid)
		{
			// Handle activation/deactivation for passive abilities

			// There is an ability in this slot. Deactivate and clear this slot
			if (!SlotIsEmpty(Slot)) 
			{
				FGameplayAbilitySpec* SpecWithSlot = GetSpecFromSlot(Slot);
				if (SpecWithSlot)
				{
					// If the ability already exists in this slot, if so clean up and return
					if (AbilityTag.MatchesTagExact(GetAbilityTagFromSpec(*SpecWithSlot)))
					{
						ClientEquipAbility(AbilityTag, GameplayTags.Abilities_Status_Equipped, Slot, PreviousSlot);
						return;
					}

					// If the ability is a passive ability
					if (IsPassiveAbility(*SpecWithSlot))
					{
						MulticastActivatePassiveEffect(GetAbilityTagFromSpec(*SpecWithSlot), false);
						DeactivatePassiveAbilityDelegate.Broadcast(GetAbilityTagFromSpec(*SpecWithSlot));
					}

					// Remove old status and add unlocked status
					SpecWithSlot->DynamicAbilityTags.RemoveTag(GetStatusFromSpec(*SpecWithSlot));
					SpecWithSlot->DynamicAbilityTags.AddTag(GameplayTags.Abilities_Status_Unlocked);

					// Clear the slot
					ClearSlot(SpecWithSlot);
				}
			}

			// Ability doesnt have a slot (Its Not Active)
			if (!AbilityHasAnySlot(*AbilitySpec))
			{
				// Checks if ability is passive
				if (IsPassiveAbility(*AbilitySpec))
				{
					TryActivateAbility(AbilitySpec->Handle);
					MulticastActivatePassiveEffect(AbilityTag, true);
				}

				// Remove old status and add unlocked status
				AbilitySpec->DynamicAbilityTags.RemoveTag(GetStatusFromSpec(*AbilitySpec));
				AbilitySpec->DynamicAbilityTags.AddTag(GameplayTags.Abilities_Status_Equipped);
			}

			// Assign Slot to the ability
			AssignSlotToAbility(*AbilitySpec, Slot);
			MarkAbilitySpecDirty(*AbilitySpec);
		}

		// Call clients EquipAbility function
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
/// Broadcasts ActivatePassiveEffectDelegate
/// </summary>
/// <param name="AbilityTag"></param>
/// <param name="bActivate"></param>
void URPGAbilitySystemComponent::MulticastActivatePassiveEffect_Implementation(const FGameplayTag& AbilityTag, bool bActivate)
{
	ActivatePassiveEffectDelegate.Broadcast(AbilityTag, bActivate);
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

/*Getters ----------------------------------------------------------------->*/
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
/// Return Slot From given AbilityTag
/// </summary>
/// <param name="AbilityTag"></param>
/// <returns></returns>
FGameplayTag URPGAbilitySystemComponent::GetSlotFromAbilityTag(const FGameplayTag& AbilityTag)
{
	if (const FGameplayAbilitySpec* Spec = GetSpecFromAbilityTag(AbilityTag))
	{
		return GetInputTagFromSpec(*Spec);
	}
	return FGameplayTag();
}

/// <summary>
/// Returns Status Tag from given Ability Tag
/// </summary>
/// <param name="AbilityTag"></param>
/// <returns></returns>
FGameplayTag URPGAbilitySystemComponent::GetStatusFromAbilityTag(const FGameplayTag& AbilityTag)
{
	if (const FGameplayAbilitySpec* Spec = GetSpecFromAbilityTag(AbilityTag))
	{
		return GetStatusFromSpec(*Spec);
	}
	return FGameplayTag();
}

/// <summary>
/// Return Input tag from given AbilityTag
/// </summary>
/// <param name="AbilityTag"></param>
/// <returns></returns>
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

/// <summary>
/// Returns Spec from given slot
/// </summary>
/// <param name="AbilityTag"></param>
/// <returns></returns>
FGameplayAbilitySpec* URPGAbilitySystemComponent::GetSpecFromSlot(const FGameplayTag& Slot)
{
	FScopedAbilityListLock ActiveScopeLock(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(Slot))
		{
			return &AbilitySpec;
		}
	}
	return nullptr;
}

/// <summary>
/// Returns the ability description from ability tag
/// </summary>
/// <param name="AbilityTag"></param>
/// <param name="OutDescription"></param>
/// <param name="OutNextLevelDescription"></param>
/// <returns></returns>
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