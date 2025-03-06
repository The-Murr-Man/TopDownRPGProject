// Copyright Kyle Murray


#include "UI/HUD/RPGHUD.h"
#include"UI/Widget/RPGUserWidget.h"
#include "UI/WidgetController/OverlayWidgetController.h"
#include "UI/WidgetController/AttributeMenuWidgetController.h"
#include "UI/WidgetController/SpellMenuWidgetController.h"

/// <summary>
/// Returns the OverlayWidgetController or creates a new one if its null
/// </summary>
/// <param name="WCParams"></param>
/// <returns></returns>
UOverlayWidgetController* ARPGHUD::GetOverlayWidgetController(const FWidgetControllerParams& WCParams)
{
	// If the controller is not yet made then make it
	if (OverlayWidgetController == nullptr)
	{
		OverlayWidgetController = NewObject<UOverlayWidgetController>(this, OverlayWidgetControllerClass);
		OverlayWidgetController->SetWidgetControllerParams(WCParams);
		OverlayWidgetController->BindCallbacksToDependencies();
	}

	// Controller already exists so return it
	return OverlayWidgetController;
}

/// <summary>
/// Returns the AttributeMenuWidgetController or creates a new one if its null
/// </summary>
/// <param name="WCParams"></param>
/// <returns></returns>
UAttributeMenuWidgetController* ARPGHUD::GetAttributeMenuWidgetController(const FWidgetControllerParams& WCParams)
{
	// If the controller is not yet made then make it
	if (AttributeMenuWidgetController == nullptr)
	{
		AttributeMenuWidgetController = NewObject<UAttributeMenuWidgetController>(this, AttributeMenuWidgetControllerClass);
		AttributeMenuWidgetController->SetWidgetControllerParams(WCParams);
		AttributeMenuWidgetController->BindCallbacksToDependencies();
	}

	// Controller already exists so return it
	return AttributeMenuWidgetController;
}

/// <summary>
/// Returns the SpellMenuWidgetController or creates a new one if its null
/// </summary>
/// <param name="WCParams"></param>
/// <returns></returns>
USpellMenuWidgetController* ARPGHUD::GetSpellMenuWidgetController(const FWidgetControllerParams& WCParams)
{
	// If the controller is not yet made then make it
	if (SpellMenuWidgetController == nullptr)
	{
		SpellMenuWidgetController = NewObject<USpellMenuWidgetController>(this, SpellMenuWidgetControllerClass);
		SpellMenuWidgetController->SetWidgetControllerParams(WCParams);
		SpellMenuWidgetController->BindCallbacksToDependencies();
	}

	// Controller already exists so return it
	return SpellMenuWidgetController;
}

/// <summary>
/// Creates the Overlay and adds it to viewport
/// </summary>
/// <param name="PC"></param>
/// <param name="PS"></param>
/// <param name="ASC"></param>
/// <param name="AS"></param>
void ARPGHUD::InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	checkf(OverlayWidgetClass, TEXT("Overlay Widget Class uninitialized, please fill out BP_RPGHUD"));
	checkf(OverlayWidgetControllerClass, TEXT("Overlay Widget Controller Class uninitialized, please fill out BP_RPGHUD"));

	UUserWidget* Widget = CreateWidget<URPGUserWidget>(GetWorld(), OverlayWidgetClass);
	OverlayWidget = Cast<URPGUserWidget>(Widget);

	const FWidgetControllerParams WidgetControllerParams(PC,PS,ASC,AS);

	UOverlayWidgetController* WidgetController = GetOverlayWidgetController(WidgetControllerParams);

	OverlayWidget->SetWidgetController(WidgetController);
	WidgetController->BroadcastInitialValues();

	Widget->AddToViewport();
}
