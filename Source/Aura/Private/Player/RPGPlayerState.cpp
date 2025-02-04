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

UAbilitySystemComponent* ARPGPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ARPGPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARPGPlayerState, Level);
	DOREPLIFETIME(ARPGPlayerState, XP);
	DOREPLIFETIME(ARPGPlayerState, AttributePoints);
	DOREPLIFETIME(ARPGPlayerState, SpellPoints);
}

void ARPGPlayerState::AddToLevel(int32 InLevel)
{
	Level += InLevel;
	OnLevelChangedDelegate.Broadcast(Level);
}

void ARPGPlayerState::SetLevel(int32 InLevel)
{
	Level = InLevel;
	OnLevelChangedDelegate.Broadcast(Level);
}

void ARPGPlayerState::AddToXP(int32 InXP)
{
	XP += InXP;
	OnXPChangedDelegate.Broadcast(XP);
}

void ARPGPlayerState::AddToAttributePoints(int32 InAttributePoints)
{
	AttributePoints += InAttributePoints;
	OnAttributePointsChangedDelegate.Broadcast(AttributePoints);
}

void ARPGPlayerState::AddToSpellPoints(int32 InSpellPoints)
{
	SpellPoints += InSpellPoints;
	OnSpellPointsChangedDelegate.Broadcast(SpellPoints);
}

void ARPGPlayerState::SetXP(int32 InXP)
{
	XP = InXP;
	OnXPChangedDelegate.Broadcast(XP);
}

void ARPGPlayerState::OnRep_Level(int32 OldLevel)
{
	OnLevelChangedDelegate.Broadcast(Level);
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
