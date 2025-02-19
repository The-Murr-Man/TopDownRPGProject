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

	/*Primary Attributes ->*/
	FGameplayTag Attributes_Primary_Strength;
	FGameplayTag Attributes_Primary_Intellegence;
	FGameplayTag Attributes_Primary_Resilience;
	FGameplayTag Attributes_Primary_Vigor;
	/*Primary Attributes End <-*/

	/*Secondary Attributes ->*/
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
	/*Secondary Attributes End <-*/

	/*Meta Attributes ->*/
	FGameplayTag Attributes_Meta_IncomingXP;
	/*Meta Attributes End <-*/

	/*Input Tags ->*/
	FGameplayTag InputTag_LMB;
	FGameplayTag InputTag_RMB;
	FGameplayTag InputTag_1;
	FGameplayTag InputTag_2;
	FGameplayTag InputTag_3;
	FGameplayTag InputTag_4;

	FGameplayTag InputTag_Passive_1;
	FGameplayTag InputTag_Passive_2;
	/*Input Tags End <-*/

	/*Player Tags ->*/
	FGameplayTag Player_Block_InputPressed;
	FGameplayTag Player_Block_InputHeld;
	FGameplayTag Player_Block_InputReleased;
	FGameplayTag Player_Block_CursorTrace;
	/*Player Tags End <-*

	/*Damage Type Tags ->*/
	FGameplayTag Damage;
	FGameplayTag Damage_Fire;
	FGameplayTag Damage_Lightning;
	FGameplayTag Damage_Arcane;
	FGameplayTag Damage_Physical;

	TMap<FGameplayTag, FGameplayTag> DamageTypesToResistances;
	/*Damage Type Tags End <-*/

	/*Damage Resistances Tags ->*/
	FGameplayTag Attributes_Resistance_Fire;
	FGameplayTag Attributes_Resistance_Lightning;
	FGameplayTag Attributes_Resistance_Arcane;
	FGameplayTag Attributes_Resistance_Physical;
	/*Damage Resistances Tags End <-*/

	/*Debuff Type Tags ->*/
	FGameplayTag Debuff_Burn;
	FGameplayTag Debuff_Stun;
	FGameplayTag Debuff_Arcane;
	FGameplayTag Debuff_Physical;

	FGameplayTag Debuff_Chance;
	FGameplayTag Debuff_Damage;
	FGameplayTag Debuff_Frequency;
	FGameplayTag Debuff_Duration;

	TMap<FGameplayTag, FGameplayTag> DamageTypesToDebuffs;
	/*Debuff Type Tags End<-*/

	/*Effect Tags ->*/
	FGameplayTag Effects_HitReact;
	/*Effect Tags <-*/

	/*Ability Tags ->*/
	FGameplayTag Abilities_None;
	FGameplayTag Abilities_Attack;
	FGameplayTag Abilities_Summon;
	FGameplayTag Abilities_Fire_Firebolt;
	FGameplayTag Abilities_Fire_FireBlast;
	FGameplayTag Abilities_Arcane_ArcaneShards;
	FGameplayTag Abilities_Lightning_Electrocute;
	FGameplayTag Abilities_HitReact;
	/*Ability Tags <-*/

	/*Passive Ability Tags ->*/
	FGameplayTag Abilities_Passive_HaloOfProtection;
	FGameplayTag Abilities_Passive_LifeSiphon;
	FGameplayTag Abilities_Passive_ManaSiphon;
	/*Passive Ability Tags <-*/

	/*Ability Status Tags ->*/
	FGameplayTag Abilities_Status_Locked;
	FGameplayTag Abilities_Status_Eligible;
	FGameplayTag Abilities_Status_Unlocked;
	FGameplayTag Abilities_Status_Equipped;
	/*Ability Status Tags End <-*/

	/*Ability Type Tags ->*/
	FGameplayTag Abilities_Type_Offensive;
	FGameplayTag Abilities_Type_Passive;
	FGameplayTag Abilities_Type_None;
	/*Ability Type Tags End <-*/

	/*Cooldown Tags ->*/
	FGameplayTag Cooldown_Fire_Firebolt;
	FGameplayTag Cooldown_Fire_Fireblast;
	FGameplayTag Cooldown_Lightning_Electrocute;
	FGameplayTag Cooldown_Arcane_ArcaneShards;
	/*Cooldown Tags <-*/

	/*Combat Socket Tags ->*/
	FGameplayTag CombatSocket_Weapon;
	FGameplayTag CombatSocket_RightHand;
	FGameplayTag CombatSocket_LeftHand;
	FGameplayTag CombatSocket_Tail;
	/*Combat Socket <-*/

	/*Montage Tags ->*/
	FGameplayTag Montage_Attack_1;
	FGameplayTag Montage_Attack_2;
	FGameplayTag Montage_Attack_3;
	FGameplayTag Montage_Attack_4;
	/*Montage Tags <-*/

	/*Gameplay Cue Tags ->*/
	FGameplayTag GameplayCue_Fireblast;
	/*Gameplay Cue Tags <-*/
private:
	static FRPGGameplayTags GameplayTags;
};