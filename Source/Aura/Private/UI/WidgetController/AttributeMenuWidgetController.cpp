// Copyright Kyle Murray


#include "UI/WidgetController/AttributeMenuWidgetController.h"
#include "AbilitySystem/Data/AttributeInfo.h"
#include "AbilitySystem/RPGAttributeSet.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "RPGGameplayTags.h"

void UAttributeMenuWidgetController::BroadcastInitialValues()
{
	URPGAttributeSet* AS = CastChecked<URPGAttributeSet>(AttributeSet);

	check(AS);

	for (auto& Pair : AS->TagsToAttributes)
	{
		FRPGAttributeInfo Info = AttributeInfo->FindAttributeInfoForTag(Pair.Key);
		Info.AttributeValue = Pair.Value().GetNumericValue(AS);
		AttributeInfoDelegate.Broadcast(Info);
	}
}

void UAttributeMenuWidgetController::BindCallbacksToDependencies()
{
	
}
