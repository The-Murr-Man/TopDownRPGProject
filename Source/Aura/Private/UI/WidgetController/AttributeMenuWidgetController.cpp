// Copyright Kyle Murray


#include "UI/WidgetController/AttributeMenuWidgetController.h"
#include "AbilitySystem/Data/AttributeInfo.h"
#include "AbilitySystem/RPGAttributeSet.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "RPGGameplayTags.h"
#include "Player/RPGPlayerState.h"

void UAttributeMenuWidgetController::BroadcastInitialValues()
{
	URPGAttributeSet* AS = CastChecked<URPGAttributeSet>(AttributeSet);

	check(AS);

	for (auto& Pair : AS->TagsToAttributes)
	{
		BroadcastAttributeInfo(Pair.Key, Pair.Value());
	}

	ARPGPlayerState* RPGPlayerState = CastChecked<ARPGPlayerState>(PlayerState);
	AttributePointsChangedDelegate.Broadcast(RPGPlayerState->GetPlayerAttributePoints());
}

void UAttributeMenuWidgetController::BindCallbacksToDependencies()
{
	URPGAttributeSet* AS = CastChecked<URPGAttributeSet>(AttributeSet);

	check(AS);
	for (auto& Pair : AS->TagsToAttributes)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Pair.Value()).AddLambda(
			[this, Pair] (const FOnAttributeChangeData& Data)
			{
				BroadcastAttributeInfo(Pair.Key, Pair.Value());
			}
		);
	}
	
	ARPGPlayerState* RPGPlayerState = CastChecked<ARPGPlayerState>(PlayerState);

	RPGPlayerState->OnAttributePointsChangedDelegate.AddLambda(
		[this](int32 Points)
		{
			AttributePointsChangedDelegate.Broadcast(Points);
		}
	);
}

void UAttributeMenuWidgetController::UpgradeAttribute(const FGameplayTag& AttributeTag)
{
	URPGAbilitySystemComponent* RPGASC = CastChecked<URPGAbilitySystemComponent>(AbilitySystemComponent);
	RPGASC->UpgradeAttribute(AttributeTag);
}

void UAttributeMenuWidgetController::BroadcastAttributeInfo(const FGameplayTag& AttributeTag, const FGameplayAttribute& Attribute)
{
	FRPGAttributeInfo Info = AttributeInfo->FindAttributeInfoForTag(AttributeTag);
	Info.AttributeValue = Attribute.GetNumericValue(AttributeSet);
	AttributeInfoDelegate.Broadcast(Info);
}
