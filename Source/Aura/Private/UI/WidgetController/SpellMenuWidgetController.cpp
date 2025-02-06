// Copyright Kyle Murray


#include "UI/WidgetController/SpellMenuWidgetController.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "Player/RPGPlayerState.h"
#include "RPGGameplayTags.h"

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
	const FRPGGameplayTags GameplayTags = FRPGGameplayTags::Get();

	const int32 SpellPoints = GetRPGPS()->GetPlayerSpellPoints();
	FGameplayTag AbilityStatus;
	FGameplayAbilitySpec* AbilitySpec = GetRPGASC()->GetSpecFromAbilityTag(AbilityTag);

	bool bTagValid = AbilityTag.IsValid();
	bool bTagNone = AbilityTag.MatchesTag(FGameplayTag::EmptyTag);
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

void USpellMenuWidgetController::SpendPointButtonPressed()
{
	if (!GetRPGASC()) return;
	GetRPGASC()->ServerSpendSpellPoint(SelectedAbility.Ability);
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
