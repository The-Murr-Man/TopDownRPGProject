// Copyright Kyle Murray


#include "Game/LoadScreenSaveGame.h"

/// <summary>
/// Gets map with given name from SavedMaps array
/// </summary>
/// <param name="InMapName"></param>
/// <returns></returns>
FSavedMap ULoadScreenSaveGame::GetSavedMapWithMapName(const FString& InMapName)
{
    for (const FSavedMap& Map : SavedMaps)
    {
        if (Map.MapAssetName == InMapName)
        {
            return Map;
        }
    }
    return FSavedMap();
}

/// <summary>
/// Returns whether map exists in SavedMaps
/// </summary>
/// <param name="InMapName"></param>
/// <returns></returns>
bool ULoadScreenSaveGame::HasMap(const FString& InMapName)
{
    for (const FSavedMap& Map : SavedMaps)
    {
        if (Map.MapAssetName == InMapName)
        {
            return true;
        }
    }
    return false;
}
