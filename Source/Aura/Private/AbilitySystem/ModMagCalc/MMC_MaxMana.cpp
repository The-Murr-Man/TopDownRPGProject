// Copyright Kyle Murray


#include "AbilitySystem/ModMagCalc/MMC_MaxMana.h"
#include "AbilitySystem/RPGAttributeSet.h"
#include "Interaction/CombatInterface.h"

UMMC_MaxMana::UMMC_MaxMana()
{
	// Captures Intellegence attribute from our attribute set
	IntellegenceDef.AttributeToCapture = URPGAttributeSet::GetIntellegenceAttribute();

	// Sets the attribute source to target
	IntellegenceDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	IntellegenceDef.bSnapshot = false;

	// Adds IntellegenceDef to array
	RelevantAttributesToCapture.Add(IntellegenceDef);
}

float UMMC_MaxMana::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	// Gather tags from source and taget
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	//

	// Creating Evaluation Parameters
	FAggregatorEvaluateParameters EvaluationParameters;

	// Setting our Evaluation Parameters source and target tags to the ones from our spec
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	// Local intellence var
	float Intellegence = 0;

	// Captures attribute magnitude
	GetCapturedAttributeMagnitude(IntellegenceDef, Spec, EvaluationParameters, Intellegence);

	// Clamp intellegence
	Intellegence = FMath::Max<float>(Intellegence, 0);

	int32 PlayerLevel = 1;

	if (Spec.GetContext().GetSourceObject()->Implements<UCombatInterface>())
	{
		PlayerLevel = ICombatInterface::Execute_GetPlayerLevel(Spec.GetContext().GetSourceObject());
	}

	return 50 + 2.5f * Intellegence + 15.f * PlayerLevel;
}
