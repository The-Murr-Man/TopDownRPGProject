// Copyright Kyle Murray


#include "UI/ViewModel/MVVM_LoadSlot.h"

/// <summary>
/// 
/// </summary>
void UMVVM_LoadSlot::InitializeSlot()
{
	const int32 WidgetSwticherIndex = LoadSlotStatus.GetValue();
	SetWidgetSwitcherIndexDelegate.Broadcast(WidgetSwticherIndex);
}

void UMVVM_LoadSlot::SetPlayerName(FString InPlayerName)
{
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

void UMVVM_LoadSlot::SetPlayerLevel(int32 InPlayerLevel)
{
	UE_MVVM_SET_PROPERTY_VALUE(PlayerLevel, InPlayerLevel);
}
