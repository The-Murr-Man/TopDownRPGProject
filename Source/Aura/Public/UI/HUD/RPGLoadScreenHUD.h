// Copyright Kyle Murray

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "RPGLoadScreenHUD.generated.h"

class ULoadScreenWidget;
class UMVVM_LoadScreen;
/**
 * 
 */
UCLASS()
class AURA_API ARPGLoadScreenHUD : public AHUD
{
	GENERATED_BODY()

public:

	// Load Screen Widget
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> LoadScreenWidgetClass;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<ULoadScreenWidget> LoadScreenWidget;

	// Load Screen View Model
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UMVVM_LoadScreen> LoadScreenViewModelClass;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UMVVM_LoadScreen> LoadScreenViewModel;
protected:
	virtual void BeginPlay() override;

private:

};
