// Copyright Kyle Murray

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/RPGWidgetController.h"
#include "AttributeMenuWidgetController.generated.h"

// Forward Declarations
struct FRPGAttributeInfo;
class UAttributeInfo;
// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAttributeInfoSigniture, const FRPGAttributeInfo&, Info);

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class AURA_API UAttributeMenuWidgetController : public URPGWidgetController
{
	GENERATED_BODY()
	
public:
	virtual void BroadcastInitialValues() override;
	virtual void BindCallbacksToDependencies() override;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FAttributeInfoSigniture AttributeInfoDelegate;

protected:

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UAttributeInfo> AttributeInfo;

	

private:
};
