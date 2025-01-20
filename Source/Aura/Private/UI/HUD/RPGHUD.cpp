// Copyright Kyle Murray


#include "UI/HUD/RPGHUD.h"
#include"UI/Widget/RPGUserWidget.h"
#include "UI/WidgetController/OverlayWidgetController.h"
#include "UI/WidgetController/AttributeMenuWidgetController.h"

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
