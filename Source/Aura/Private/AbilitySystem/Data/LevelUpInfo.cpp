// Copyright Kyle Murray


#include "AbilitySystem/Data/LevelUpInfo.h"

int32 ULevelUpInfo::FindLevelForXP(int32 XP)
{
    int32 Level = 1;
    bool bSearching = true;

    while (bSearching)
    {
        // LevelUpInformation[1] = Level 1 Information ignoring [0]
        if (LevelUpInformation.Num() - 1 <= Level) return Level;

        // 
        if (XP >= LevelUpInformation[Level].LevelUpRequirment)
        {
            ++Level;
        }

        else
            bSearching = false;
    }

    return Level;
}
