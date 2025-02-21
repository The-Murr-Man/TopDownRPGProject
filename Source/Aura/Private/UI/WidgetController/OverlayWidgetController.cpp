// Copyright Kyle Murray


#include "UI/WidgetController/OverlayWidgetController.h"
#include "AbilitySystem/RPGAttributeSet.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "Player/RPGPlayerState.h"
#include "AbilitySystem/Data/LevelUpInfo.h"
#include "RPGGameplayTags.h"

/// <summary>
/// 
/// </summary>
void UOverlayWidgetController::BroadcastInitialValues()
{
	OnHealthChanged.Broadcast(GetRPGAS()->GetHealth());
	OnMaxHealthChanged.Broadcast(GetRPGAS()->GetMaxHealth());

	OnManaChanged.Broadcast(GetRPGAS()->GetMana());
	OnMaxManaChanged.Broadcast(GetRPGAS()->GetMaxMana());
}

/// <summary>
/// 
/// </summary>
void UOverlayWidgetController::BindCallbacksToDependencies()
{
	// Callback for changing xp
	GetRPGPS()->OnXPChangedDelegate.AddUObject(this, &UOverlayWidgetController::OnXPChanged);

	GetRPGPS()->OnLevelChangedDelegate.AddLambda(
		[this](int32 NewLevel, bool bLevelUp)
		{
			OnPlayerLevelChangedDelegate.Broadcast(NewLevel, bLevelUp);
		}
	);
	
	BindAttributeChange(GetRPGAS()->GetHealthAttribute(), OnHealthChanged);
	BindAttributeChange(GetRPGAS()->GetMaxHealthAttribute(), OnMaxHealthChanged);
	BindAttributeChange(GetRPGAS()->GetManaAttribute(), OnManaChanged);
	BindAttributeChange(GetRPGAS()->GetMaxManaAttribute(), OnMaxManaChanged);

	if (GetRPGASC())
	{
		GetRPGASC()->AbilityEquippedDelegate.AddUObject(this, &UOverlayWidgetController::OnAbilityEquipped);

		if (GetRPGASC()->bStartupAbilitiesGiven)
			BroadcastAbilityInfo();
		else
			GetRPGASC()->AbilitiesGivenDelegate.AddUObject(this,&UOverlayWidgetController::BroadcastAbilityInfo);

		// Using a lamda function to loop through all tags on the ability system component
		GetRPGASC()->EffectAssetTags.AddLambda(
			[this](const FGameplayTagContainer& AssetTags)
			{
				for (const FGameplayTag& Tag : AssetTags)
				{
					FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(FName("Message"));
					if (!Tag.MatchesTag(MessageTag)) return;

					FUIWidgetRow* Row = GetDataTableRowByTag<FUIWidgetRow>(MessageWidgetDataTable, Tag);
					MessegeWidgetRowDelegate.Broadcast(*Row);
				}
			}
		);
	}
	
}

/// <summary>
/// 
/// </summary>
/// <param name="Attribute"></param>
/// <param name="AttributeData"></param>
void UOverlayWidgetController::BindAttributeChange(FGameplayAttribute Attribute, FOnAttributeChangedSignature& AttributeData)
{
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Attribute).AddLambda(
		[this, &AttributeData](const FOnAttributeChangeData& Data)
		{
			AttributeData.Broadcast(Data.NewValue);
		}
	);
}

/// <summary>
/// 
/// </summary>
/// <param name="NewXP"></param>
void UOverlayWidgetController::OnXPChanged(int32 NewXP)
{
	ULevelUpInfo* LevelUpInfo = GetRPGPS()->LevelUpInfo;

	checkf(LevelUpInfo, TEXT("Unable to find LevelUpInfo, Please fill our RPGPlayerState Blueprint"));

	int32 Level = LevelUpInfo->FindLevelForXP(NewXP);
	int32 MaxLevel = LevelUpInfo->LevelUpInformation.Num();

	if (Level <= MaxLevel && Level > 0)
	{
		int32 LevelUpRequirment = LevelUpInfo->LevelUpInformation[Level].LevelUpRequirment;
		int32 PreviousLevelUpRequirment = LevelUpInfo->LevelUpInformation[Level - 1].LevelUpRequirment;

		int32 DeltaLevelUpRequirment = LevelUpRequirment - PreviousLevelUpRequirment;
		int32 XPForThisLevel = NewXP - PreviousLevelUpRequirment;

		float XPBarPercent = static_cast<float>(XPForThisLevel) / static_cast<float>(DeltaLevelUpRequirment);

		OnXPPercentChangedDelegate.Broadcast(XPBarPercent);
	}
}

void UOverlayWidgetController::OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& Status, const FGameplayTag& Slot, const FGameplayTag& PreviousSlot)
{
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
}
