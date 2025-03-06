// Copyright Kyle Murray


#include "UI/ViewModel/MVVM_LoadScreen.h"
#include "UI/ViewModel/MVVM_LoadSlot.h"
#include "Game/RPGGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Game/RPGGameInstance.h"

/// <summary>
/// Settup load slots
/// </summary>
void UMVVM_LoadScreen::InitializeLoadSlots()
{
	// Creating new Objects for each LoadSlot and adding them to our map
	LoadSlot_0 = NewObject<UMVVM_LoadSlot>(this, LoadSlotViewModelClass);
	LoadSlot_0->SetLoadSlotName(FString("LoadSlot_0"));
	LoadSlot_0->SlotIndex = 0;
	LoadSlots.Add(0, LoadSlot_0);

	LoadSlot_1 = NewObject<UMVVM_LoadSlot>(this, LoadSlotViewModelClass);
	LoadSlot_1->SetLoadSlotName(FString("LoadSlot_1"));
	LoadSlot_1->SlotIndex = 1;
	LoadSlots.Add(1, LoadSlot_1);

	LoadSlot_2 = NewObject<UMVVM_LoadSlot>(this, LoadSlotViewModelClass);
	LoadSlot_2->SetLoadSlotName(FString("LoadSlot_2"));
	LoadSlot_2->SlotIndex = 2;
	LoadSlots.Add(2, LoadSlot_2);

	SetNumLoadSlots(LoadSlots.Num());
}

/// <summary>
/// Returns the LoadSlot at given index
/// </summary>
/// <param name="Index"></param>
/// <returns></returns>
const UMVVM_LoadSlot* UMVVM_LoadScreen::GetLoadSlotViewModelByIndex(int32 Index)
{
	return LoadSlots.FindChecked(Index);
}

/// <summary>
/// Handles functionality when the new slot button is pressed
/// </summary>
/// <param name="Slot"></param>
/// <param name="EnteredName"></param>
void UMVVM_LoadScreen::NewSlotButtonPressed(int32 Slot, const FString& EnteredName)
{
	ARPGGameModeBase* RPGGameMode = Cast<ARPGGameModeBase>(UGameplayStatics::GetGameMode(this));

	if (!IsValid(RPGGameMode)) return; // Turns off multiplayer

	// Sets the slots information
	LoadSlots[Slot]->SetPlayerName(EnteredName);
	LoadSlots[Slot]->LoadSlotStatus = Taken;
	LoadSlots[Slot]->SetMapName(RPGGameMode->DefaultMapName);
	LoadSlots[Slot]->PlayerStartTag = RPGGameMode->DefaultPlayerStartTag;
	LoadSlots[Slot]->SetPlayerLevel(1);
	LoadSlots[Slot]->SetMapAssetName(RPGGameMode->DefaultMap.ToSoftObjectPath().GetAssetName());
	
	// Saves the slots data
	RPGGameMode->SaveSlotData(LoadSlots[Slot], Slot);

	// Initial the slot
	LoadSlots[Slot]->InitializeSlot();

	URPGGameInstance* RGPGameInstance = Cast<URPGGameInstance>(RPGGameMode->GetGameInstance());

	if (!IsValid(RGPGameInstance)) return;

	// Sets slots information from game instance
	RGPGameInstance->LoadSlotName = LoadSlots[Slot]->GetLoadSlotName();
	RGPGameInstance->LoadSlotIndex = LoadSlots[Slot]->SlotIndex;
	RGPGameInstance->PlayerStartTag = RPGGameMode->DefaultPlayerStartTag;
}

/// <summary>
/// Handles functionality when the new game button is pressed
/// </summary>
/// <param name="Slot"></param>
void UMVVM_LoadScreen::NewGameButtonPressed(int32 Slot)
{
	LoadSlots[Slot]->SetWidgetSwitcherIndexDelegate.Broadcast(1);
}

/// <summary>
/// Handles functionality when the select slot button is pressed
/// </summary>
/// <param name="Slot"></param>
void UMVVM_LoadScreen::SelectSlotButtonPressed(int32 Slot)
{
	SlotSelected.Broadcast();

	// Loop through LoadSlots array
	for (const TTuple<int32, UMVVM_LoadSlot*> LoadSlot : LoadSlots)
	{
		// Set button disabled when pressed
		if (LoadSlot.Key == Slot)
		{
			LoadSlot.Value->EnableSelectSlotButtonDelegate.Broadcast(false);
		}
		
		// Set other buttons enabled when button pressed
		else
		{
			LoadSlot.Value->EnableSelectSlotButtonDelegate.Broadcast(true);
		}
	}

	// Sets the selected slot
	SelectedSlot = LoadSlots[Slot];
}

/// <summary>
///  Handles functionality when the Delete slot button is pressed
/// </summary>
void UMVVM_LoadScreen::DeleteButtonPressed()
{
	if (!IsValid(SelectedSlot)) return;

	// Delete the slot
	ARPGGameModeBase::DeleteSlot(SelectedSlot->GetLoadSlotName(), SelectedSlot->SlotIndex);

	// Update the widget
	SelectedSlot->LoadSlotStatus = Vacant;
	SelectedSlot->InitializeSlot();
	SelectedSlot->EnableSelectSlotButtonDelegate.Broadcast(true);
}

/// <summary>
///  Handles functionality when the Play slot button is pressed
/// </summary>
/// <param name="Slot"></param>
void UMVVM_LoadScreen::PlayButtonPressed()
{
	// Get the GameMode and GameInstance
	ARPGGameModeBase* RPGGameMode = Cast<ARPGGameModeBase>(UGameplayStatics::GetGameMode(this));
	URPGGameInstance* RGPGameInstance = Cast<URPGGameInstance>(RPGGameMode->GetGameInstance());

	// Check if GM and GI are valid
	if (!IsValid(RPGGameMode) && !IsValid(RGPGameInstance)) return;

	// Sets GameInstace information from selected slot
	RGPGameInstance->PlayerStartTag = SelectedSlot->PlayerStartTag;
	RGPGameInstance->LoadSlotName = SelectedSlot->GetLoadSlotName();
	RGPGameInstance->LoadSlotIndex = SelectedSlot->SlotIndex;
	if (!IsValid(SelectedSlot)) return;

	// Travel to selected slots map
	RPGGameMode->TravelToMap(SelectedSlot);
}

/// <summary>
/// Gets the Save slot data from the GameMode and sets values loaded in
/// </summary>
void UMVVM_LoadScreen::LoadData()
{
	ARPGGameModeBase* RPGGameMode = Cast<ARPGGameModeBase>(UGameplayStatics::GetGameMode(this));

	if (!IsValid(RPGGameMode)) return;

	// Loops through LoadSlots Array
	for (const TTuple<int32, UMVVM_LoadSlot*> LoadSlot : LoadSlots)
	{
		// Get the SaveObject from the GameMode
		ULoadScreenSaveGame* SaveObject = RPGGameMode->GetSaveSlotData(LoadSlot.Value->GetLoadSlotName(), LoadSlot.Key);

		// Setting Values loaded in from the save object
		const FString PlayerName = SaveObject->PlayerName;
		const FString MapName = SaveObject->MapName;
		TEnumAsByte<ESaveSlotStatus> SaveSlotStatus = SaveObject->SaveSlotStatus;

		// Set values on load slot
		LoadSlot.Value->LoadSlotStatus = SaveSlotStatus;
		LoadSlot.Value->SetPlayerName(PlayerName);
		LoadSlot.Value->SetPlayerLevel(SaveObject->PlayerLevel);
		LoadSlot.Value->InitializeSlot();

		LoadSlot.Value->SetMapName(MapName);
		LoadSlot.Value->PlayerStartTag = SaveObject->PlayerStartTag;
	}
}

/// <summary>
/// Sets the number of load slots available
/// </summary>
/// <param name="InNumLoadSlots"></param>
void UMVVM_LoadScreen::SetNumLoadSlots(int32 InNumLoadSlots)
{
	UE_MVVM_SET_PROPERTY_VALUE(NumLoadSlots, InNumLoadSlots);
}
