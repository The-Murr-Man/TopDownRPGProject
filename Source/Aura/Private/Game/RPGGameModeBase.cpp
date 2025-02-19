// Copyright Kyle Murray


#include "Game/RPGGameModeBase.h"
#include "Game/LoadScreenSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "UI/ViewModel/MVVM_LoadSlot.h"

void ARPGGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	GameMaps.Add(DefaultMapName, DefaultMap);
}


/// <summary>
/// 
/// </summary>
/// <param name="LoadSlot"></param>
/// <param name="SlotIndex"></param>
void ARPGGameModeBase::SaveSlotData(UMVVM_LoadSlot* LoadSlot, int32 SlotIndex)
{
	// Checking to see if save game exists for given load slot
	if (UGameplayStatics::DoesSaveGameExist(LoadSlot->GetLoadSlotName(), SlotIndex))
	{
		// If load slot already exists, delete it
		UGameplayStatics::DeleteGameInSlot(LoadSlot->GetLoadSlotName(), SlotIndex);
	}

	USaveGame* SaveGameObject = UGameplayStatics::CreateSaveGameObject(LoadScreenSaveGameClass);
	ULoadScreenSaveGame* LoadScreenSaveGame = Cast<ULoadScreenSaveGame>(SaveGameObject);
	LoadScreenSaveGame->PlayerName = LoadSlot->GetPlayerName();
	LoadScreenSaveGame->SaveSlotStatus = Taken;
	LoadScreenSaveGame->MapName = LoadSlot->GetMapName();

	UGameplayStatics::SaveGameToSlot(LoadScreenSaveGame, LoadSlot->GetLoadSlotName(), SlotIndex);
}

/// <summary>
/// 
/// </summary>
/// <param name="SlotName"></param>
/// <param name="SlotIndex"></param>
void ARPGGameModeBase::DeleteSlot(const FString& SlotName, int32 SlotIndex)
{
	// Checking to see if save game exists for given load slot
	if (UGameplayStatics::DoesSaveGameExist(SlotName, SlotIndex))
	{
		// If load slot already exists, delete it
		UGameplayStatics::DeleteGameInSlot(SlotName, SlotIndex);
	}
}

/// <summary>
/// Travels to given map
/// </summary>
/// <param name="LoadSlot"></param>
void ARPGGameModeBase::TravelToMap(UMVVM_LoadSlot* LoadSlot)
{
	const FString SlotName = LoadSlot->GetLoadSlotName();
	const int32 SlotIndex = LoadSlot->SlotIndex;
	UGameplayStatics::OpenLevelBySoftObjectPtr(LoadSlot, GameMaps.FindChecked(LoadSlot->GetMapName()));
}

/// <summary>
/// 
/// </summary>
/// <param name="SlotName"></param>
/// <param name="SlotIndex"></param>
/// <returns></returns>
ULoadScreenSaveGame* ARPGGameModeBase::GetSaveSlotData(const FString& SlotName, int32 SlotIndex) const
{
	USaveGame* SaveGameObject = nullptr;
	// Checking to see if save game exists for given load slot
	if (UGameplayStatics::DoesSaveGameExist(SlotName, SlotIndex))
	{
		// If load slot already exists, load it
		SaveGameObject = UGameplayStatics::LoadGameFromSlot(SlotName, SlotIndex);
	}

	else
	{
		// If no save game exists create one
		SaveGameObject = UGameplayStatics::CreateSaveGameObject(LoadScreenSaveGameClass);
	}
	
	ULoadScreenSaveGame* LoadScreenSaveGame = Cast<ULoadScreenSaveGame>(SaveGameObject);

	return LoadScreenSaveGame;
}

