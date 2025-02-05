// Copyright Kyle Murray


#include "UI/WidgetController/SpellMenuWidgetController.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "AbilitySystem/Data/AbilityInfo.h"

void USpellMenuWidgetController::BroadcastInitialValues()
{
	BroadcastAbilityInfo();
}

void USpellMenuWidgetController::BindCallbacksToDependencies()
{
	GetRPGASC()->AbilityStatusChangedDelegate.AddLambda(
		[this](const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag)
		{
			if (AbilityInfo)
			{
				FRPGAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
				Info.StatusTag = StatusTag;
				AbilityInfoDelegate.Broadcast(Info);
			}
		});
}
