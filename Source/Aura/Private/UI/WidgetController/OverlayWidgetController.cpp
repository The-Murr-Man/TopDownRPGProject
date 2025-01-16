// Copyright Kyle Murray


#include "UI/WidgetController/OverlayWidgetController.h"
#include "AbilitySystem/RPGAttributeSet.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"

void UOverlayWidgetController::BroadcastInitialValues()
{
	const URPGAttributeSet* RPGAttributeSet = CastChecked<URPGAttributeSet>(AttributeSet);

	
	OnHealthChanged.Broadcast(RPGAttributeSet->GetHealth());
	OnMaxHealthChanged.Broadcast(RPGAttributeSet->GetMaxHealth());

	OnManaChanged.Broadcast(RPGAttributeSet->GetMana());
	OnMaxManaChanged.Broadcast(RPGAttributeSet->GetMaxMana());
}

void UOverlayWidgetController::BindCallbacksToDependencies()
{
	const URPGAttributeSet* RPGAttributeSet = CastChecked<URPGAttributeSet>(AttributeSet);

	BindAttributeChange(RPGAttributeSet->GetHealthAttribute(), OnHealthChanged);
	BindAttributeChange(RPGAttributeSet->GetMaxHealthAttribute(), OnMaxHealthChanged);
	BindAttributeChange(RPGAttributeSet->GetManaAttribute(), OnManaChanged);
	BindAttributeChange(RPGAttributeSet->GetMaxManaAttribute(), OnMaxManaChanged);

	// Using a lamda function to loop through all tags on the ability system component
	Cast<URPGAbilitySystemComponent>(AbilitySystemComponent)->EffectAssetTags.AddLambda(
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

void UOverlayWidgetController::BindAttributeChange(FGameplayAttribute Attribute, FOnAttributeChangedSignature& AttributeData)
{
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Attribute).AddLambda(
		[this, &AttributeData](const FOnAttributeChangeData& Data)
		{
			AttributeData.Broadcast(Data.NewValue);
		}
	);
}
