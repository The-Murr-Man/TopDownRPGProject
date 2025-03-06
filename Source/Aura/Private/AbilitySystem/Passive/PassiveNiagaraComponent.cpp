// Copyright Kyle Murray


#include "AbilitySystem/Passive/PassiveNiagaraComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "Interaction/CombatInterface.h"
#include "RPGGameplayTags.h"

UPassiveNiagaraComponent::UPassiveNiagaraComponent()
{
	bAutoActivate = false;
}

void UPassiveNiagaraComponent::BeginPlay()
{
	Super::BeginPlay();

	if (URPGAbilitySystemComponent* RPGASC = Cast<URPGAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner())))
	{
		RPGASC->ActivatePassiveEffectDelegate.AddUObject(this, &UPassiveNiagaraComponent::OnPassiveActivate);
		const bool bStartupAbilitiesGiven = RPGASC->bStartupAbilitiesGiven;

		ActivateIfEquipped(RPGASC);
	}

	else if(ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetOwner()))
	{
		CombatInterface->GetOnASCRegisteredDelegate().AddLambda(
			[this](UAbilitySystemComponent* ASC)
			{
				if (URPGAbilitySystemComponent* RPGASC = Cast<URPGAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner())))
				{
					RPGASC->ActivatePassiveEffectDelegate.AddUObject(this, &UPassiveNiagaraComponent::OnPassiveActivate);
					ActivateIfEquipped(RPGASC);
				}
			});
	}
}

/// <summary>
/// Activate Passive Effect
/// </summary>
/// <param name="AbilityTag"></param>
/// <param name="bActivate"></param>
void UPassiveNiagaraComponent::OnPassiveActivate(const FGameplayTag& AbilityTag, bool bActivate)
{
	if (AbilityTag.MatchesTagExact(PassiveSpellTag))
	{
		if (bActivate && !IsActive())
		{
			Activate();
		}

		else
		{
			Deactivate();
		}
	}
}

/// <summary>
/// Auto active if ability is equipped
/// </summary>
/// <param name="RPGASC"></param>
void UPassiveNiagaraComponent::ActivateIfEquipped(URPGAbilitySystemComponent* RPGASC)
{
	const bool bStartupAbilitiesGiven = RPGASC->bStartupAbilitiesGiven;

	if (bStartupAbilitiesGiven)
	{
		if (RPGASC->GetStatusFromAbilityTag(PassiveSpellTag) == FRPGGameplayTags::Get().Abilities_Status_Equipped)
		{
			Activate();
		}
	}
}
