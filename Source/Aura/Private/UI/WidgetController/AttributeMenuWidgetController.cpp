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

	AttributePointsChangedDelegate.Broadcast(GetRPGPS()->GetPlayerAttributePoints());
}

void UAttributeMenuWidgetController::BindCallbacksToDependencies()
{

	check(GetRPGAS());
	for (auto& Pair : GetRPGAS()->TagsToAttributes)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Pair.Value()).AddLambda(
			[this, Pair] (const FOnAttributeChangeData& Data)
			{
				BroadcastAttributeInfo(Pair.Key, Pair.Value());
			}
		);
	}

	GetRPGPS()->OnAttributePointsChangedDelegate.AddLambda(
		[this](int32 Points)
		{
			AttributePointsChangedDelegate.Broadcast(Points);
		}
	);
}

void UAttributeMenuWidgetController::UpgradeAttribute(const FGameplayTag& AttributeTag)
{
	GetRPGASC()->UpgradeAttribute(AttributeTag);
}

void UAttributeMenuWidgetController::BroadcastAttributeInfo(const FGameplayTag& AttributeTag, const FGameplayAttribute& Attribute)
{
	FRPGAttributeInfo Info = AttributeInfo->FindAttributeInfoForTag(AttributeTag);
	Info.AttributeValue = Attribute.GetNumericValue(AttributeSet);
	AttributeInfoDelegate.Broadcast(Info);
}
