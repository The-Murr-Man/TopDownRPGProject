// Copyright Kyle Murray

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RPGGameModeBase.generated.h"

class UCharacterClassInfo;
class UAbilityInfo;
class UMVVM_LoadSlot;
class USaveGame;
class ULoadScreenSaveGame;

/**
 * 
 */
UCLASS()
class AURA_API ARPGGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditDefaultsOnly, Category = "Character Class Defaults")
	TObjectPtr<UCharacterClassInfo> CharacterClassInfo;

	UPROPERTY(EditDefaultsOnly, Category = "Ability Info")
	TObjectPtr<UAbilityInfo> AbilityInfo;

	UPROPERTY(EditDefaultsOnly, Category = "Save Game")
	TSubclassOf<USaveGame> LoadScreenSaveGameClass;

	UPROPERTY(EditDefaultsOnly, Category = "Game Maps")
	FString DefaultMapName;

	UPROPERTY(EditDefaultsOnly, Category = "Game Maps")
	TSoftObjectPtr<UWorld> DefaultMap;

	UPROPERTY(EditDefaultsOnly, Category = "Game Maps")
	FName DefaultPlayerStartTag;

	UPROPERTY(EditDefaultsOnly, Category = "Game Maps")
	TMap<FString, TSoftObjectPtr<UWorld>> GameMaps;

	FString GetMapNameFromMapAssetName(const FString& MapAssetName);

	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	ULoadScreenSaveGame* GetSaveSlotData(const FString& SlotName, int32 SlotIndex) const;

	ULoadScreenSaveGame* RetreiveInGameSaveData();

	void SaveInGameProgressData(ULoadScreenSaveGame* SaveObject);

	void SaveSlotData(UMVVM_LoadSlot* LoadSlot, int32 SlotIndex);

	static void DeleteSlot(const FString& SlotName, int32 SlotIndex);

	void TravelToMap(UMVVM_LoadSlot* LoadSlot);

	void SaveWorldState(UWorld* World, const FString& DestinationMapAssetName = FString(""));
	void LoadWorldState(UWorld* World);

protected:

	virtual void BeginPlay() override;
};
