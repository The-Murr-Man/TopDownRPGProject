// Copyright Kyle Murray

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "RPGHUD.generated.h"

class URPGUserWidget;
class UOverlayWidgetController;
class UAttributeMenuWidgetController;
struct FWidgetControllerParams;
class UAbilitySystemComponent;
class UAttributeSet;

/**
 * 
 */
UCLASS()
class AURA_API ARPGHUD : public AHUD
{
	GENERATED_BODY()
	
public:

	UOverlayWidgetController* GetOverlayWidgetController(const FWidgetControllerParams& WCParams);
	UAttributeMenuWidgetController* GetAttributeMenuWidgetController(const FWidgetControllerParams& WCParams);

	void InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS);

private:

	// Overlay Widget Controller
	UPROPERTY()
	TObjectPtr<URPGUserWidget> OverlayWidget;

	UPROPERTY(EditAnywhere)
	TSubclassOf<URPGUserWidget> OverlayWidgetClass;

	UPROPERTY()
	TObjectPtr<UOverlayWidgetController> OverlayWidgetController;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UOverlayWidgetController> OverlayWidgetControllerClass;
	//

	// Attribute MenuWidget Controller
	UPROPERTY()
	TObjectPtr<UAttributeMenuWidgetController> AttributeMenuWidgetController;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UAttributeMenuWidgetController> AttributeMenuWidgetControllerClass;
	//
protected:
	


};
