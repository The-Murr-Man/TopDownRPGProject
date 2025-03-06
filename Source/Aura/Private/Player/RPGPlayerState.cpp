// Copyright Kyle Murray


#include "Player/RPGPlayerState.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "AbilitySystem/RPGAttributeSet.h"
#include "Net/UnrealNetwork.h"

ARPGPlayerState::ARPGPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<URPGAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<URPGAttributeSet>("AttributeSet");

	// How often the server will update
	NetUpdateFrequency = 100.f;
}

/// <summary>
/// Returns the AbilitySystemComponent
/// </summary>
/// <returns></returns>
UAbilitySystemComponent* ARPGPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

/// <summary>
/// Used to replicate Variables
/// </summary>
/// <param name="OutLifetimeProps"></param>
void ARPGPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// MACRO for replicating variables
	DOREPLIFETIME(ARPGPlayerState, Level);
	DOREPLIFETIME(ARPGPlayerState, XP);
	DOREPLIFETIME(ARPGPlayerState, AttributePoints);
	DOREPLIFETIME(ARPGPlayerState, SpellPoints);
}

/// <summary>
/// Adds given Level to player level and broadcasts the change
/// </summary>
/// <param name="InLevel"></param>
void ARPGPlayerState::AddToLevel(int32 InLevel)
{
	Level += InLevel;
	OnLevelChangedDelegate.Broadcast(Level,true);
}

/// <summary>
/// Sets player level and broadcasts the change
/// </summary>
/// <param name="InLevel"></param>
void ARPGPlayerState::SetPlayerLevel(int32 InLevel)
{
	Level = InLevel;
	OnLevelChangedDelegate.Broadcast(Level,false);
}

/// <summary>
/// Sets player attribute points and broadcasts the change
/// </summary>
/// <param name="InAttributePoints"></param>
void ARPGPlayerState::SetPlayerAttributePoints(int32 InAttributePoints)
{
	AttributePoints = InAttributePoints;
	OnAttributePointsChangedDelegate.Broadcast(AttributePoints);
}

/// <summary>
/// Sets player spell points and broadcasts the change
/// </summary>
/// <param name="InSpellPoints"></param>
void ARPGPlayerState::SetPlayerSpellPoints(int32 InSpellPoints)
{
	SpellPoints = InSpellPoints;
	OnSpellPointsChangedDelegate.Broadcast(SpellPoints);
}

/// <summary>
/// Adds to players Experience and broadcasts the change
/// </summary>
/// <param name="InXP"></param>
void ARPGPlayerState::AddToXP(int32 InXP)
{
	XP += InXP;
	OnXPChangedDelegate.Broadcast(XP);
}

/// <summary>
/// Adds to attributes points and broadcasts the change
/// </summary>
/// <param name="InAttributePoints"></param>
void ARPGPlayerState::AddToAttributePoints(int32 InAttributePoints)
{
	AttributePoints += InAttributePoints;
	OnAttributePointsChangedDelegate.Broadcast(AttributePoints);
}

/// <summary>
/// Adds to spell points and broadcasts the change
/// </summary>
/// <param name="InSpellPoints"></param>
void ARPGPlayerState::AddToSpellPoints(int32 InSpellPoints)
{
	SpellPoints += InSpellPoints;
	OnSpellPointsChangedDelegate.Broadcast(SpellPoints);
}

/// <summary>
/// Sets players Experience and broadcasts the change
/// </summary>
/// <param name="InXP"></param>
void ARPGPlayerState::SetPlayerXP(int32 InXP)
{
	XP = InXP;
	OnXPChangedDelegate.Broadcast(XP);
}

// Replicated Functions
void ARPGPlayerState::OnRep_Level(int32 OldLevel)
{
	OnLevelChangedDelegate.Broadcast(Level,true);
}

void ARPGPlayerState::OnRep_XP(int32 OldXP)
{
	OnXPChangedDelegate.Broadcast(XP);
}

void ARPGPlayerState::OnRep_AttributePoints(int32 OldAttributePoints)
{
	OnAttributePointsChangedDelegate.Broadcast(AttributePoints);
}

void ARPGPlayerState::OnRep_SpellPoints(int32 OldSpellPoints)
{
	OnSpellPointsChangedDelegate.Broadcast(SpellPoints);
}
