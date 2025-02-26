// Copyright Kyle Murray


#include "Game/RPGGameModeBase.h"
#include "Game/LoadScreenSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "UI/ViewModel/MVVM_LoadSlot.h"
#include "GameFramework/PlayerStart.h"
#include "Game/RPGGameInstance.h"
#include "Interaction/SaveInterface.h"
#include "EngineUtils.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "GameFramework/Character.h"

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
	LoadScreenSaveGame->PlayerStartTag = LoadSlot->PlayerStartTag;
	LoadScreenSaveGame->MapAssetName = LoadSlot->GetMapAssetName();
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
/// Saves to given world to disk
/// </summary>
/// <param name="World"></param>
void ARPGGameModeBase::SaveWorldState(UWorld* World, const FString& DestinationMapAssetName)
{
	FString WorldName = World->GetMapName();

	// Removes the StreamingLevelsPrefix from the world name so we just have the normal name
	WorldName.RemoveFromStart(World->StreamingLevelsPrefix);

	URPGGameInstance* RGPGameInstance = Cast<URPGGameInstance>(GetGameInstance());
	check(RGPGameInstance);

	if (ULoadScreenSaveGame* SaveGame = GetSaveSlotData(RGPGameInstance->LoadSlotName, RGPGameInstance->LoadSlotIndex))
	{
		if (DestinationMapAssetName != FString(""))
		{
			SaveGame->MapAssetName = DestinationMapAssetName;
			SaveGame->MapName = GetMapNameFromMapAssetName(DestinationMapAssetName);
		}

		// Checks if Map has already been saved
		if (!SaveGame->HasMap(WorldName))
		{
			FSavedMap NewSaveMap;
			NewSaveMap.MapAssetName = WorldName;
			SaveGame->SavedMaps.Add(NewSaveMap);
		}

		FSavedMap SavedMap = SaveGame->GetSavedMapWithMapName(WorldName);
		SavedMap.SavedActors.Empty(); // Clear it out, we will fill it in with "actors"

		// Iterates over all actors in the world
		for (FActorIterator It(World); It; ++It)
		{
			AActor* Actor = *It;

			if (!IsValid(Actor) || !Actor->Implements<USaveInterface>()) continue;

			FSavedActor SavedActor;
			SavedActor.ActorName = Actor->GetFName();
			SavedActor.Transform = Actor->GetTransform();

			// Used to Serialize Data in memory
			FMemoryWriter MemoryWriter(SavedActor.Bytes);

			// Creating an archive with our data
			FObjectAndNameAsStringProxyArchive Archive(MemoryWriter, true);
			Archive.ArIsSaveGame = true;
			
			// Serializes our Actor into the archive
			Actor->Serialize(Archive);

			SavedMap.SavedActors.AddUnique(SavedActor);
		}

		// Replace saved map
		for (FSavedMap& MapToReplace : SaveGame->SavedMaps)
		{
			if (MapToReplace.MapAssetName == WorldName)
			{
				MapToReplace = SavedMap;
			}
		}

		UGameplayStatics::SaveGameToSlot(SaveGame, RGPGameInstance->LoadSlotName, RGPGameInstance->LoadSlotIndex);
	}
}

/// <summary>
/// Loads the given world from disk
/// </summary>
/// <param name="World"></param>
void ARPGGameModeBase::LoadWorldState(UWorld* World)
{
	FString WorldName = World->GetMapName();

	// Removes the StreamingLevelsPrefix from the world name so we just have the normal name
	WorldName.RemoveFromStart(World->StreamingLevelsPrefix);

	URPGGameInstance* RGPGameInstance = Cast<URPGGameInstance>(GetGameInstance());
	check(RGPGameInstance);

	FString LoadSlotName = RGPGameInstance->LoadSlotName;
	int32 LoadSlotIndex = RGPGameInstance->LoadSlotIndex;

	if (UGameplayStatics::DoesSaveGameExist(LoadSlotName, LoadSlotIndex))
	{
		ULoadScreenSaveGame* SaveGame = Cast<ULoadScreenSaveGame>(UGameplayStatics::LoadGameFromSlot(LoadSlotName, LoadSlotIndex));

		if (!SaveGame)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed To Load Slot"));
			return;
		}

		// Iterates over all actors in the world
		for (FActorIterator It(World); It; ++It)
		{
			AActor* Actor = *It;

			if (!Actor->Implements<USaveInterface>()) continue;

			for (FSavedActor SavedActor : SaveGame->GetSavedMapWithMapName(WorldName).SavedActors)
			{
				if (SavedActor.ActorName == Actor->GetFName())
				{
					if (ISaveInterface::Execute_ShouldLoadTransform(Actor))
					{
						Actor->SetActorTransform(SavedActor.Transform);
					}
				}

				// Used to Read Serialized Data from memory
				FMemoryReader MemoryReader(SavedActor.Bytes);

				// Creating an archive with our data
				FObjectAndNameAsStringProxyArchive Archive(MemoryReader, true);
				Archive.ArIsSaveGame = true;

				// Deserialize actor (Still uses the Serialize Function), converts binary bytes back into variables
				Actor->Serialize(Archive);

				// Calls the Interface load actor function
				ISaveInterface::Execute_LoadActor(Actor);
			}
		}
	}
}

/// <summary>
/// Handles functionality of players death
/// </summary>
/// <param name="DeadCharacter"></param>
void ARPGGameModeBase::PlayerDied(ACharacter* DeadCharacter)
{
	ULoadScreenSaveGame* SaveGame = RetreiveInGameSaveData();

	if (!IsValid(SaveGame)) return;

	UGameplayStatics::OpenLevel(DeadCharacter, FName(SaveGame->MapAssetName));

}

/// <summary>
/// Loops through GameMaps and get the MapName from a given map asset name
/// </summary>
/// <param name="MapAssetName"></param>
/// <returns></returns>
FString ARPGGameModeBase::GetMapNameFromMapAssetName(const FString& MapAssetName)
{
	for (auto& Map : GameMaps)
	{
		if (Map.Value.ToSoftObjectPath().GetAssetName() == MapAssetName)
		{
			return Map.Key;
		}
	}
	return FString();
}

/// <summary>
/// 
/// </summary>
/// <param name="Player"></param>
/// <returns></returns>
AActor* ARPGGameModeBase::ChoosePlayerStart_Implementation(AController* Player)
{
	URPGGameInstance* RGPGameInstance = Cast<URPGGameInstance>(GetGameInstance());

	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), Actors);

	if (Actors.Num() > 0)
	{
		AActor* SelectedActor = Actors[0];

		for (AActor* Actor : Actors)
		{
			if (APlayerStart* PlayerStart = Cast<APlayerStart>(Actor))
			{
				if (PlayerStart->PlayerStartTag == RGPGameInstance->PlayerStartTag)
				{
					SelectedActor = PlayerStart;
					break;
				}
			}
		}

		return SelectedActor;
	}
	return nullptr;
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

/// <summary>
/// Helper function used to retrieve all the save data
/// </summary>
/// <returns></returns>
ULoadScreenSaveGame* ARPGGameModeBase::RetreiveInGameSaveData()
{
	URPGGameInstance* RGPGameInstance = Cast<URPGGameInstance>(GetGameInstance());
	const FString InGameLoadSlotName = RGPGameInstance->LoadSlotName;
	const int32 InGameLoadSlotIndex = RGPGameInstance->LoadSlotIndex;

	return GetSaveSlotData(InGameLoadSlotName,InGameLoadSlotIndex);
}

/// <summary>
/// Used to Save Progress Data In Game
/// </summary>
/// <param name="SaveObject"></param>
void ARPGGameModeBase::SaveInGameProgressData(ULoadScreenSaveGame* SaveObject)
{
	URPGGameInstance* RGPGameInstance = Cast<URPGGameInstance>(GetGameInstance());
	const FString InGameLoadSlotName = RGPGameInstance->LoadSlotName;
	const int32 InGameLoadSlotIndex = RGPGameInstance->LoadSlotIndex;

	RGPGameInstance->PlayerStartTag = SaveObject->PlayerStartTag;

	UGameplayStatics::SaveGameToSlot(SaveObject, InGameLoadSlotName, InGameLoadSlotIndex);
}

