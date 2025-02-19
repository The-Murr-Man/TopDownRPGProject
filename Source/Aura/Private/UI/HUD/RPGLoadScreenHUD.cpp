// Copyright Kyle Murray


#include "UI/HUD/RPGLoadScreenHUD.h"
#include "Blueprint/UserWidget.h"
#include "UI/ViewModel/MVVM_LoadScreen.h"
#include "UI/Widget/LoadScreenWidget.h"

void ARPGLoadScreenHUD::BeginPlay()
{
	Super::BeginPlay();

	// Creating our loadscreen view model
	LoadScreenViewModel = NewObject<UMVVM_LoadScreen>(this, LoadScreenViewModelClass);

	// Initialize the load slots
	LoadScreenViewModel->InitializeLoadSlots();

	// Creating a new widget using our widget class
	LoadScreenWidget = CreateWidget<ULoadScreenWidget>(GetWorld(), LoadScreenWidgetClass);

	// Adding widget to the viewport
	LoadScreenWidget->AddToViewport();

	//Used to access the load slot widgets and set there view models
	LoadScreenWidget->BlueprintInitializeWidget();

	// Load save game data to the view model
	LoadScreenViewModel->LoadData();
}
