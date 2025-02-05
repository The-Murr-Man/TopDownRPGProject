// Copyright Kyle Murray


#include "UI/WidgetController/RPGWidgetController.h"
#include "Player/RPGPlayerController.h"
#include "Player/RPGPlayerState.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "AbilitySystem/RPGAttributeSet.h"
#include "AbilitySystem/Data/AbilityInfo.h"

void URPGWidgetController::SetWidgetControllerParams(const FWidgetControllerParams& WCParams)
{
	PlayerController = WCParams.PlayerController;
	PlayerState = WCParams.PlayerState;
	AttributeSet = WCParams.AttributeSet;
	AbilitySystemComponent = WCParams.AbilitySystemComponent;
}

void URPGWidgetController::BroadcastInitialValues()
{

}

void URPGWidgetController::BindCallbacksToDependencies()
{

}

void URPGWidgetController::BroadcastAbilityInfo()
{
	if (!GetRPGASC()->bStartupAbilitiesGiven) return;

	FForEachAbility BroadcastDelegate;
	BroadcastDelegate.BindLambda([this](const FGameplayAbilitySpec& AbilitySpec)
		{
			FRPGAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(GetRPGASC()->GetAbilityTagFromSpec(AbilitySpec));
			Info.InputTag = GetRPGASC()->GetInputTagFromSpec(AbilitySpec);
			Info.StatusTag = GetRPGASC()->GetStatusFromSpec(AbilitySpec);
			AbilityInfoDelegate.Broadcast(Info);
		});

	GetRPGASC()->ForEachAbility(BroadcastDelegate);
}

/// <summary>
/// Returns the Player Controller
/// </summary>
/// <returns></returns>
ARPGPlayerController* URPGWidgetController::GetRPGPC()
{
	if (RPGPlayerController == nullptr)
	{
		RPGPlayerController = Cast<ARPGPlayerController>(PlayerController);
	}

	return RPGPlayerController;
}

/// <summary>
/// Returns the Player State
/// </summary>
/// <returns></returns>
ARPGPlayerState* URPGWidgetController::GetRPGPS()
{
	if (RPGPlayerState == nullptr)
	{
		RPGPlayerState = Cast<ARPGPlayerState>(PlayerState);
	}

	return RPGPlayerState;
}

/// <summary>
/// Returns the Ability System Component
/// </summary>
/// <returns></returns>
URPGAbilitySystemComponent* URPGWidgetController::GetRPGASC()
{
	if (RPGAbilitySystemComponent == nullptr)
	{
		RPGAbilitySystemComponent = Cast<URPGAbilitySystemComponent>(AbilitySystemComponent);
	}

	return RPGAbilitySystemComponent;
}

/// <summary>
/// Returns the Attribute Set
/// </summary>
/// <returns></returns>
URPGAttributeSet* URPGWidgetController::GetRPGAS()
{
	if (RPGAttributeSet == nullptr)
	{
		RPGAttributeSet = Cast<URPGAttributeSet>(AttributeSet);
	}

	return RPGAttributeSet;
}
