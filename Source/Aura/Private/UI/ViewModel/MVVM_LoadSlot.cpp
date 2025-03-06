// Copyright Kyle Murray


#include "UI/ViewModel/MVVM_LoadSlot.h"

void UMVVM_LoadSlot::InitializeSlot()
{
	const int32 WidgetSwticherIndex = LoadSlotStatus.GetValue();
	SetWidgetSwitcherIndexDelegate.Broadcast(WidgetSwticherIndex);
}

void UMVVM_LoadSlot::SetPlayerName(FString InPlayerName)
{
	// Setter function for a Model View View Model Property value
	UE_MVVM_SET_PROPERTY_VALUE(PlayerName, InPlayerName);
}

void UMVVM_LoadSlot::SetLoadSlotName(FString InLoadSlotName)
{
	UE_MVVM_SET_PROPERTY_VALUE(LoadSlotName, InLoadSlotName);
}

void UMVVM_LoadSlot::SetMapName(FString InMapName)
{
	UE_MVVM_SET_PROPERTY_VALUE(MapName, InMapName);
}

void UMVVM_LoadSlot::SetMapAssetName(FString InMapAssetName)
{
	UE_MVVM_SET_PROPERTY_VALUE(MapAssetName, InMapAssetName);
}

void UMVVM_LoadSlot::SetPlayerLevel(int32 InPlayerLevel)
{
	UE_MVVM_SET_PROPERTY_VALUE(PlayerLevel, InPlayerLevel);
}
