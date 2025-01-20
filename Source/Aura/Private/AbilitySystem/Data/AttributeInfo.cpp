// Copyright Kyle Murray


#include "AbilitySystem/Data/AttributeInfo.h"

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
		UE_LOG(LogTemp, Error, TEXT("Cant find Info for AttributeTag [%s] on AttributeInfo [%s]."), *AttributeTag.ToString(), *GetNameSafe(this));
	}

	return FRPGAttributeInfo();
}
