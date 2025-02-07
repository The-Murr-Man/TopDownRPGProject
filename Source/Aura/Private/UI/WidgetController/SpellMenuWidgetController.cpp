// Copyright Kyle Murray


#include "UI/WidgetController/SpellMenuWidgetController.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "Player/RPGPlayerState.h"
#include "RPGGameplayTags.h"
#include "GameplayTagContainer.h"

/// <summary>
/// 
/// </summary>
void USpellMenuWidgetController::BroadcastInitialValues()
{
	BroadcastAbilityInfo();

	// Broadcasting Initial Spell Points
	SpellPointsChangedDelegate.Broadcast(GetRPGPS()->GetPlayerSpellPoints());
}

/// <summary>
/// 
/// </summary>
void USpellMenuWidgetController::BindCallbacksToDependencies()
{
	GetRPGASC()->AbilityStatusChangedDelegate.AddLambda(
		[this](const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag,int32 NewLevel)
		{
			if (SelectedAbility.Ability.MatchesTagExact(AbilityTag))
			{
				SelectedAbility.Status = StatusTag;

				bool bEnableSpendPoints = false;
				bool bEnableEquip = false;
				ShouldEnableButtons(StatusTag, CurrentSpellPoints, bEnableSpendPoints, bEnableEquip);

				FString Description;
				FString NextLevelDescription;
				GetRPGASC()->GetDescriptionsByAbilityTag(AbilityTag, Description,NextLevelDescription);
				
				SpellGlobeSelectedDelegate.Broadcast(bEnableSpendPoints, bEnableEquip, Description,NextLevelDescription);
			}

			if (AbilityInfo)
			{
				FRPGAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
				Info.StatusTag = StatusTag;
				AbilityInfoDelegate.Broadcast(Info);
			}
		});

	GetRPGASC()->AbilityEquippedDelegate.AddUObject(this, &USpellMenuWidgetController::OnAbilityEquipped);

	//Lambda For updating spell points
	GetRPGPS()->OnSpellPointsChangedDelegate.AddLambda(
		[this](int SpellPoints)
		{
			SpellPointsChangedDelegate.Broadcast(SpellPoints);
			CurrentSpellPoints = SpellPoints;

			bool bEnableSpendPoints = false;
			bool bEnableEquip = false;

			ShouldEnableButtons(SelectedAbility.Status, CurrentSpellPoints, bEnableSpendPoints, bEnableEquip);
			
			FString Description;
			FString NextLevelDescription;
			GetRPGASC()->GetDescriptionsByAbilityTag(SelectedAbility.Ability, Description, NextLevelDescription);

			SpellGlobeSelectedDelegate.Broadcast(bEnableSpendPoints, bEnableEquip, Description, NextLevelDescription);
		});
}

/// <summary>
/// 
/// </summary>
/// <param name="AbilityTag"></param>
void USpellMenuWidgetController::SpellGlobeSelected(const FGameplayTag& AbilityTag)
{
	if (bWaitingForEquipSelection)
	{
		FGameplayTag SelectedAbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.Ability).AbilityType;

		StopWaitingForEquipDelegate.Broadcast(SelectedAbilityType);
		bWaitingForEquipSelection = false;
	}
	

	const FRPGGameplayTags GameplayTags = FRPGGameplayTags::Get();

	const int32 SpellPoints = GetRPGPS()->GetPlayerSpellPoints();
	FGameplayTag AbilityStatus;
	FGameplayAbilitySpec* AbilitySpec = GetRPGASC()->GetSpecFromAbilityTag(AbilityTag);

	bool bTagValid = AbilityTag.IsValid();
	bool bTagNone = AbilityTag.MatchesTag(GameplayTags.Abilities_None);
	bool bSpecValid = AbilitySpec != nullptr;

	// Set Status Locked
	if (!bTagValid|| bTagNone ||!bSpecValid)
	{
		AbilityStatus = GameplayTags.Abilities_Status_Locked;
	}

	else
	{
		AbilityStatus = GetRPGASC()->GetStatusFromSpec(*AbilitySpec);
	}

	SelectedAbility.Ability = AbilityTag;
	SelectedAbility.Status = AbilityStatus;

	bool bEnableSpendPoints = false;
	bool bEnableEquip = false;

	ShouldEnableButtons(AbilityStatus, SpellPoints, bEnableSpendPoints, bEnableEquip);
	

	FString Description;
	FString NextLevelDescription;
	GetRPGASC()->GetDescriptionsByAbilityTag(SelectedAbility.Ability, Description, NextLevelDescription);

	SpellGlobeSelectedDelegate.Broadcast(bEnableSpendPoints, bEnableEquip, Description, NextLevelDescription);
}

/// <summary>
/// 
/// </summary>
void USpellMenuWidgetController::SpellGlobeDeselected()
{
	if (bWaitingForEquipSelection)
	{
		FGameplayTag SelectedAbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.Ability).AbilityType;

		StopWaitingForEquipDelegate.Broadcast(SelectedAbilityType);
		bWaitingForEquipSelection = false;
	}

	SelectedAbility.Ability = FRPGGameplayTags::Get().Abilities_None;
	SelectedAbility.Status = FRPGGameplayTags::Get().Abilities_Status_Locked;

	SpellGlobeSelectedDelegate.Broadcast(false, false , FString(), FString());
}

/// <summary>
/// 
/// </summary>
void USpellMenuWidgetController::SpendPointButtonPressed()
{
	if (!GetRPGASC()) return;
	GetRPGASC()->ServerSpendSpellPoint(SelectedAbility.Ability);
}

/// <summary>
/// 
/// </summary>
void USpellMenuWidgetController::EquipButtonPressed()
{
	const FGameplayTag AbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.Ability).AbilityType;
	WaitForEquipDelegate.Broadcast(AbilityType);
	bWaitingForEquipSelection = true;

	const FGameplayTag SelectedStatus = GetRPGASC()->GetStatusFromAbilityTag(SelectedAbility.Ability);

	if (SelectedStatus.MatchesTagExact(FRPGGameplayTags::Get().Abilities_Status_Equipped))
	{
		SelectedSlot = GetRPGASC()->GetInputTagFromAbilityTag(SelectedAbility.Ability);
	}
}

/// <summary>
/// 
/// </summary>
/// <param name="SlotTag - Input Tag for the Slot"></param>
/// <param name="AbilityType - Type of ability in the Slot"></param>
void USpellMenuWidgetController::SpellRowGlobePressed(const FGameplayTag& SlotTag, const FGameplayTag& AbilityType)
{
	if (!bWaitingForEquipSelection) return;
	
	// Check Selected ability against slot's ability type. (Dont equip an offensive spell in passive slot and vice versa)
	const FGameplayTag SelectedAbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.Ability).AbilityType;
	if (!SelectedAbilityType.MatchesTagExact(AbilityType)) return;

	GetRPGASC()->ServerEquipAbility(SelectedAbility.Ability, SlotTag);
}

/// <summary>
/// 
/// </summary>
/// <param name="AbilityTag"></param>
/// <param name="Status"></param>
/// <param name="Slot"></param>
/// <param name="PreviousSlot"></param>
void USpellMenuWidgetController::OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& Status, const FGameplayTag& Slot, const FGameplayTag& PreviousSlot)
{
	bWaitingForEquipSelection = false;

	const FRPGGameplayTags& GameplayTags = FRPGGameplayTags::Get();

	// Clear out LastSlot
	FRPGAbilityInfo LastSlotInfo;

	LastSlotInfo.StatusTag = GameplayTags.Abilities_Status_Unlocked;
	LastSlotInfo.InputTag = PreviousSlot;
	LastSlotInfo.AbilityTag = GameplayTags.Abilities_None;

	// Broadcast empty info if PreviousSlot is a valud slot. Only if equipping and already-equipped spell
	AbilityInfoDelegate.Broadcast(LastSlotInfo);
	
	// Fill CurrentSlot with new info
	FRPGAbilityInfo CurrentSlotInfo = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
	CurrentSlotInfo.StatusTag = Status;
	CurrentSlotInfo.InputTag = Slot;

	AbilityInfoDelegate.Broadcast(CurrentSlotInfo);

	StopWaitingForEquipDelegate.Broadcast(AbilityInfo->FindAbilityInfoForTag(AbilityTag).AbilityType);

	SpellGlobeDeselected();
}

/// <summary>
/// 
/// </summary>
/// <param name="AbilityStatus"></param>
/// <param name="SpellPoints"></param>
/// <param name="bShouldEnableSpendPointsButton"></param>
/// <param name="bShouldEnableEquipButton"></param>
void USpellMenuWidgetController::ShouldEnableButtons(const FGameplayTag& AbilityStatus, int32 SpellPoints, bool& bShouldEnableSpendPointsButton, bool& bShouldEnableEquipButton)
{
	const FRPGGameplayTags GameplayTags = FRPGGameplayTags::Get();

	bShouldEnableSpendPointsButton = false;
	bShouldEnableEquipButton = false;

	if (AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Equipped) || AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Unlocked))
	{
		bShouldEnableEquipButton = true;
		
		if (SpellPoints >0 )
		{
			bShouldEnableSpendPointsButton = true;
		}
	}

	else if (AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Eligible))
	{
		if (SpellPoints > 0)
		{
			bShouldEnableSpendPointsButton = true;
		}
	}
}
