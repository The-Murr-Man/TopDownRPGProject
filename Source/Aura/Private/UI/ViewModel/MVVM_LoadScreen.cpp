// Copyright Kyle Murray


#include "UI/ViewModel/MVVM_LoadScreen.h"
#include "UI/ViewModel/MVVM_LoadSlot.h"
#include "Game/RPGGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Game/RPGGameInstance.h"

/// <summary>
/// 
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
/// 
/// </summary>
/// <param name="Index"></param>
/// <returns></returns>
const UMVVM_LoadSlot* UMVVM_LoadScreen::GetLoadSlotViewModelByIndex(int32 Index)
{
	return LoadSlots.FindChecked(Index);
}

/// <summary>
/// 
/// </summary>
/// <param name="Slot"></param>
/// <param name="EnteredName"></param>
void UMVVM_LoadScreen::NewSlotButtonPressed(int32 Slot, const FString& EnteredName)
{
	ARPGGameModeBase* RPGGameMode = Cast<ARPGGameModeBase>(UGameplayStatics::GetGameMode(this));

	if (!IsValid(RPGGameMode)) return; // Turns off multiplayer

	LoadSlots[Slot]->SetPlayerName(EnteredName);
	LoadSlots[Slot]->LoadSlotStatus = Taken;
	LoadSlots[Slot]->SetMapName(RPGGameMode->DefaultMapName);
	LoadSlots[Slot]->PlayerStartTag = RPGGameMode->DefaultPlayerStartTag;
	LoadSlots[Slot]->SetPlayerLevel(1);
	LoadSlots[Slot]->SetMapAssetName(RPGGameMode->DefaultMap.ToSoftObjectPath().GetAssetName());
	
	RPGGameMode->SaveSlotData(LoadSlots[Slot], Slot);
	LoadSlots[Slot]->InitializeSlot();

	URPGGameInstance* RGPGameInstance = Cast<URPGGameInstance>(RPGGameMode->GetGameInstance());

	if (!IsValid(RGPGameInstance)) return;

	RGPGameInstance->LoadSlotName = LoadSlots[Slot]->GetLoadSlotName();
	RGPGameInstance->LoadSlotIndex = LoadSlots[Slot]->SlotIndex;
	RGPGameInstance->PlayerStartTag = RPGGameMode->DefaultPlayerStartTag;
}

/// <summary>
/// 
/// </summary>
/// <param name="Slot"></param>
void UMVVM_LoadScreen::NewGameButtonPressed(int32 Slot)
{
	LoadSlots[Slot]->SetWidgetSwitcherIndexDelegate.Broadcast(1);
}

/// <summary>
/// 
/// </summary>
/// <param name="Slot"></param>
void UMVVM_LoadScreen::SelectSlotButtonPressed(int32 Slot)
{
	SlotSelected.Broadcast();

	for (const TTuple<int32, UMVVM_LoadSlot*> LoadSlot : LoadSlots)
	{
		// Set button disabled when pressed
		if (LoadSlot.Key == Slot)
		{
			LoadSlot.Value->EnableSelectSlotButtonDelegate.Broadcast(false);
		}
		
		// Set other buttones enabled when button pressed
		else
		{
			LoadSlot.Value->EnableSelectSlotButtonDelegate.Broadcast(true);
		}
	}

	// Sets the selected slot
	SelectedSlot = LoadSlots[Slot];
}

/// <summary>
/// 
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
/// 
/// </summary>
/// <param name="Slot"></param>
void UMVVM_LoadScreen::PlayButtonPressed()
{
	ARPGGameModeBase* RPGGameMode = Cast<ARPGGameModeBase>(UGameplayStatics::GetGameMode(this));
	URPGGameInstance* RGPGameInstance = Cast<URPGGameInstance>(RPGGameMode->GetGameInstance());
	if (!IsValid(RPGGameMode) && !IsValid(RGPGameInstance)) return;

	RGPGameInstance->PlayerStartTag = SelectedSlot->PlayerStartTag;
	RGPGameInstance->LoadSlotName = SelectedSlot->GetLoadSlotName();
	RGPGameInstance->LoadSlotIndex = SelectedSlot->SlotIndex;
	if (!IsValid(SelectedSlot)) return;

	RPGGameMode->TravelToMap(SelectedSlot);
}

/// <summary>
/// Gets the Save slot data from the GameMode and sets values loaded in
/// </summary>
void UMVVM_LoadScreen::LoadData()
{
	ARPGGameModeBase* RPGGameMode = Cast<ARPGGameModeBase>(UGameplayStatics::GetGameMode(this));

	if (!IsValid(RPGGameMode)) return;

	for (const TTuple<int32, UMVVM_LoadSlot*> LoadSlot : LoadSlots)
	{
		ULoadScreenSaveGame* SaveObject = RPGGameMode->GetSaveSlotData(LoadSlot.Value->GetLoadSlotName(), LoadSlot.Key);

		// Setting Values loaded in from the save object
		const FString PlayerName = SaveObject->PlayerName;
		const FString MapName = SaveObject->MapName;
		TEnumAsByte<ESaveSlotStatus> SaveSlotStatus = SaveObject->SaveSlotStatus;

		LoadSlot.Value->LoadSlotStatus = SaveSlotStatus;
		LoadSlot.Value->SetPlayerName(PlayerName);
		LoadSlot.Value->SetPlayerLevel(SaveObject->PlayerLevel);
		LoadSlot.Value->InitializeSlot();

		LoadSlot.Value->SetMapName(MapName);
		LoadSlot.Value->PlayerStartTag = SaveObject->PlayerStartTag;
	}
}

/// <summary>
/// 
/// </summary>
/// <param name="InNumLoadSlots"></param>
void UMVVM_LoadScreen::SetNumLoadSlots(int32 InNumLoadSlots)
{
	UE_MVVM_SET_PROPERTY_VALUE(NumLoadSlots, InNumLoadSlots);
}
