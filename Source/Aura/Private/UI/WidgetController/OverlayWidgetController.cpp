// Copyright Kyle Murray


#include "UI/WidgetController/OverlayWidgetController.h"
#include "AbilitySystem/RPGAttributeSet.h"

void UOverlayWidgetController::BroadcastInitialValues()
{
	const URPGAttributeSet* RPGAttributeSet = CastChecked<URPGAttributeSet>(AttributeSet);

	OnHealthChanged.Broadcast(RPGAttributeSet->GetHealth());
	OnMaxHealthChanged.Broadcast(RPGAttributeSet->GetMaxHealth());
	
}
