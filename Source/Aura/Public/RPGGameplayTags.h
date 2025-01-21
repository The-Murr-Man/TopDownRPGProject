// Copyright Kyle Murray

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/*
* RPGGameplayTags
* 
* Singleton containing native Gameplay tags
*/

struct FRPGGameplayTags
{

public:
	static const FRPGGameplayTags& Get() { return GameplayTags; };

	static void InitializeNativeGameplayTags();

	/*
	* Primary Attributes ->
	*/
	FGameplayTag Attributes_Primary_Strength;
	FGameplayTag Attributes_Primary_Intellegence;
	FGameplayTag Attributes_Primary_Resilience;
	FGameplayTag Attributes_Primary_Vigor;
	/*
	* Primary Attributes End <-
	*/

	/*
	* Secondary Attributes ->
	*/
	FGameplayTag Attributes_Secondary_MaxHealth;
	FGameplayTag Attributes_Secondary_MaxMana;
	FGameplayTag Attributes_Secondary_Armor;
	FGameplayTag Attributes_Secondary_ArmorPenetration;
	FGameplayTag Attributes_Secondary_BlockChance;
	FGameplayTag Attributes_Secondary_CritcalHitChance;
	FGameplayTag Attributes_Secondary_CritcalHitDamage;
	FGameplayTag Attributes_Secondary_CritcalHitResistance;
	FGameplayTag Attributes_Secondary_HealthRegeneration;
	FGameplayTag Attributes_Secondary_ManaRegeneration;
	/*
	* Secondary Attributes End <-
	*/

	/*
	* Input Tags ->
	*/
	FGameplayTag InputTag_LMB;
	FGameplayTag InputTag_RMB;
	FGameplayTag InputTag_1;
	FGameplayTag InputTag_2;
	FGameplayTag InputTag_3;
	FGameplayTag InputTag_4;
	/*
	* Input Tags End <-
	*/

private:
	static FRPGGameplayTags GameplayTags;
};