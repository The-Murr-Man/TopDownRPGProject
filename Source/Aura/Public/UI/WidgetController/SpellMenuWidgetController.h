// Copyright Kyle Murray

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/RPGWidgetController.h"
#include "SpellMenuWidgetController.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API USpellMenuWidgetController : public URPGWidgetController
{
	GENERATED_BODY()
	
public:
	virtual void BroadcastInitialValues() override;
	virtual void BindCallbacksToDependencies() override;
};
