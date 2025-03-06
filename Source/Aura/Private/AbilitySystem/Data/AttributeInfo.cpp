// Copyright Kyle Murray


#include "AbilitySystem/Data/AttributeInfo.h"
#include "RPGLogChannels.h"

/// <summary>
/// Gets attribute info for given tag
/// </summary>
/// <param name="AttributeTag"></param>
/// <param name="bLogNotFound"></param>
/// <returns></returns>
FRPGAttributeInfo UAttributeInfo::FindAttributeInfoForTag(const FGameplayTag& AttributeTag, bool bLogNotFound) const
{
	for (const FRPGAttributeInfo Info : AttributeInformation)
	{
		if (Info.AttributeTag == AttributeTag)
		{
			return Info;
		}
	}
	
	if (bLogNotFound)
	{
		UE_LOG(LogRPG, Error, TEXT("Cant find Info for AttributeTag [%s] on AttributeInfo [%s]."), *AttributeTag.ToString(), *GetNameSafe(this));
	}

	return FRPGAttributeInfo();
}
