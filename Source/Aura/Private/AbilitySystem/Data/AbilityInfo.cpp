// Copyright Kyle Murray


#include "AbilitySystem/Data/AbilityInfo.h"
#include "RPGLogChannels.h"

FRPGAbilityInfo UAbilityInfo::FindAbilityInfoForTag(const FGameplayTag& AbilityTag, bool bLogNotFound) const
{
    for (const FRPGAbilityInfo& Info : AbilityInformation)
    {
        if (Info.AbilityTag == AbilityTag) return Info;
    }

    if (bLogNotFound)
    {
        UE_LOG(LogRPG, Error, TEXT("Cant find info for AbilityTag[%s] on AbilityInfo [%s]"), *AbilityTag.ToString(),*GetNameSafe(this));
    }

    return FRPGAbilityInfo();
}
