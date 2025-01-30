// Copyright Kyle Murray


#include "AbilitySystem/ExecCalc/ExecCalc_Damage.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/RPGAttributeSet.h"
#include "RPGGameplayTags.h"
#include "Interaction/CombatInterface.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "AbilitySystem/RPGAbilitySystemLibrary.h"
#include "RPGAbilityTypes.h"

struct RPGDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor);
	DECLARE_ATTRIBUTE_CAPTUREDEF(BlockChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ArmorPenetration);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitDamage);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitResistance);

	DECLARE_ATTRIBUTE_CAPTUREDEF(ArcaneResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(FireResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(LightningResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(PhysicalResistance);

	TMap<FGameplayTag, FGameplayEffectAttributeCaptureDefinition> TagsToCaptureDefs;

	RPGDamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(URPGAttributeSet, Armor, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(URPGAttributeSet, BlockChance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(URPGAttributeSet, ArmorPenetration, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(URPGAttributeSet, CriticalHitChance, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(URPGAttributeSet, CriticalHitDamage, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(URPGAttributeSet, CriticalHitResistance, Target, false);

		DEFINE_ATTRIBUTE_CAPTUREDEF(URPGAttributeSet, ArcaneResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(URPGAttributeSet, FireResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(URPGAttributeSet, LightningResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(URPGAttributeSet, PhysicalResistance, Target, false);


		TagsToCaptureDefs.Add(FRPGGameplayTags::Get().Attributes_Secondary_Armor, ArmorDef);
		TagsToCaptureDefs.Add(FRPGGameplayTags::Get().Attributes_Secondary_BlockChance, BlockChanceDef);
		TagsToCaptureDefs.Add(FRPGGameplayTags::Get().Attributes_Secondary_ArmorPenetration, ArmorPenetrationDef);
		TagsToCaptureDefs.Add(FRPGGameplayTags::Get().Attributes_Secondary_CritcalHitChance, CriticalHitChanceDef);
		TagsToCaptureDefs.Add(FRPGGameplayTags::Get().Attributes_Secondary_CritcalHitDamage, CriticalHitDamageDef);
		TagsToCaptureDefs.Add(FRPGGameplayTags::Get().Attributes_Secondary_CritcalHitResistance, CriticalHitResistanceDef);

		TagsToCaptureDefs.Add(FRPGGameplayTags::Get().Attributes_Resistance_Arcane, ArcaneResistanceDef);
		TagsToCaptureDefs.Add(FRPGGameplayTags::Get().Attributes_Resistance_Fire, FireResistanceDef);
		TagsToCaptureDefs.Add(FRPGGameplayTags::Get().Attributes_Resistance_Lightning, LightningResistanceDef);
		TagsToCaptureDefs.Add(FRPGGameplayTags::Get().Attributes_Resistance_Physical, PhysicalResistanceDef);
	}
};

static const RPGDamageStatics& DamageStatics()
{
	static RPGDamageStatics DStatics;
	return DStatics;
}

UExecCalc_Damage::UExecCalc_Damage()
{
	RelevantAttributesToCapture.Add(DamageStatics().ArmorDef);
	RelevantAttributesToCapture.Add(DamageStatics().BlockChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().ArmorPenetrationDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitDamageDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitResistanceDef);

	RelevantAttributesToCapture.Add(DamageStatics().ArcaneResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().FireResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().LightningResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().PhysicalResistanceDef);
}

void UExecCalc_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	// Gets the SourceASC from ExecutionParams
	const UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();

	// Gets the TargetASC from ExecutionParams
	const UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

	AActor* SourceAvatar = SourceASC ? SourceASC->GetAvatarActor() : nullptr;

	AActor* TargetAvatar = TargetASC ? TargetASC->GetAvatarActor() : nullptr;

	// Get spec from ExecutionParams
	const FGameplayEffectSpec Spec = ExecutionParams.GetOwningSpec();

	// Gather tags from source and taget
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	// Create Evaluation Params
	FAggregatorEvaluateParameters EvaluationParameters;

	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	// Get the combat interfaces from our source and target avatars
	ICombatInterface* SourceCombatInterface = Cast<ICombatInterface>(SourceAvatar);
	ICombatInterface* TargetCombatInterface = Cast<ICombatInterface>(TargetAvatar);

	FGameplayEffectContextHandle EffectContextHandle = Spec.GetContext();
	//////////////////////////////////////////////

	// Get Damage Set By Caller Magnitude
	float Damage = 0;

	for (const auto& Pair : FRPGGameplayTags::Get().DamageTypesToResistances)
	{
		const FGameplayTag DamageType = Pair.Key;
		const FGameplayTag ResistanceTag = Pair.Value;

		checkf(RPGDamageStatics().TagsToCaptureDefs.Contains(ResistanceTag), TEXT("TagsToCaptureDefs doesn't containg tag [%s] in ExecCalc_Damage"), *ResistanceTag.ToString());
		const FGameplayEffectAttributeCaptureDefinition CaptureDef = RPGDamageStatics().TagsToCaptureDefs[ResistanceTag];

		float DamageTypeValue = Spec.GetSetByCallerMagnitude(Pair.Key,false,0);

		float Resistance = 0;
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(CaptureDef, EvaluationParameters, Resistance);
		Resistance = FMath::Clamp(Resistance,0, 75);

		DamageTypeValue *= (100 - Resistance) / 100;
		
		Damage += DamageTypeValue;
	}

	// Capture Block Chance on target and Determine if there was a successful block ->
	float TargetBlockChance = 0;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().BlockChanceDef, EvaluationParameters, TargetBlockChance);
	TargetBlockChance = FMath::Max<float>(TargetBlockChance,0);

	const bool bBlocked = FMath::RandRange(1, 100) < TargetBlockChance;
	// If Block, halve the damage

	//  
	URPGAbilitySystemLibrary::SetIsBlockedHit(EffectContextHandle, bBlocked);

	Damage = bBlocked ? Damage *= 0.5 : Damage;

	/// <-

	// Capture the targets Armor ->
	float TargetArmor = 0;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorDef, EvaluationParameters, TargetArmor);
	TargetArmor = FMath::Max<float>(TargetArmor, 0);
	// <-

	// Capture the Sources ArmorPenetration ->
	float SourceArmorPenetration = 0;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorPenetrationDef, EvaluationParameters, SourceArmorPenetration);
	SourceArmorPenetration = FMath::Max<float>(SourceArmorPenetration, 0);
	// <-

	// Getting the CharacterClassInfo from our blueprint system library
	UCharacterClassInfo* CharacterClassInfo = URPGAbilitySystemLibrary::GetCharacterClassInfo(SourceAvatar);

	// Get ArmorPenetrationCurve from our data asset
	FRealCurve* ArmorPenetrationCurve = CharacterClassInfo->DamageCalculationCoefficients->FindCurve(FName("ArmorPenetration"), FString());

	// Gets the ArmorPenetrationCoefficient from the curve based on player level
	const float ArmorPenetrationCoefficient = ArmorPenetrationCurve->Eval(SourceCombatInterface->GetPlayerLevel());

	// ArmorPenetration ignores a percentage of targets armor
	const float EffectiveArmor = TargetArmor * (100 - SourceArmorPenetration * ArmorPenetrationCoefficient) / 100;

	// Get EffectiveArmorCurve from our data asset
	FRealCurve* EffectiveArmorCurve = CharacterClassInfo->DamageCalculationCoefficients->FindCurve(FName("EffectiveArmor"), FString());

	// Gets the EffectiveArmorCoefficient from the curve based on player level
	const float EffectiveArmorCoefficient = EffectiveArmorCurve->Eval(SourceCombatInterface->GetPlayerLevel());

	// Armor ignores a precentage of incoming damage
	Damage *= (100 - EffectiveArmor * EffectiveArmorCoefficient) / 100;

	// Captures the sources CriticalHitChance ->
	float SourceCriticalHitChance = 0;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitChanceDef, EvaluationParameters, SourceCriticalHitChance);
	SourceCriticalHitChance = FMath::Max<float>(SourceCriticalHitChance, 0);
	// <-

	// Captures the sources SourceCriticalHitDamage ->
	float SourceCriticalHitDamage = 0;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitDamageDef, EvaluationParameters, SourceCriticalHitDamage);
	SourceCriticalHitDamage = FMath::Max<float>(SourceCriticalHitDamage, 0);
	// <-

	// Captures the sources TargetCriticalHitResistance ->
	float TargetCriticalHitResistance = 0;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitResistanceDef, EvaluationParameters, TargetCriticalHitResistance);
	TargetCriticalHitResistance = FMath::Max<float>(TargetCriticalHitResistance, 0);
	// <-


	// Get ArmorPenetrationCurve from our data asset
	FRealCurve * CriticalHitResistanceCurve = CharacterClassInfo->DamageCalculationCoefficients->FindCurve(FName("CriticalHitResistance"), FString());

	// Gets the ArmorPenetrationCoefficient from the curve based on player level
	const float CriticalHitResistanceCoefficient = CriticalHitResistanceCurve->Eval(SourceCombatInterface->GetPlayerLevel());

	// Critical hit chance calculation, Critical Hit Resistance reduces Critical Hit Chance by a certain percentage
	const float EffectiveCriticalHitChance = SourceCriticalHitChance - TargetCriticalHitResistance * CriticalHitResistanceCoefficient;
	const bool bCriticalHit = FMath::RandRange(1, 100) < EffectiveCriticalHitChance;

	URPGAbilitySystemLibrary::SetIsCriticalHit(EffectContextHandle, bCriticalHit);

	// If crit hit scale damage by double and add SourceCriticalHitDamage
	Damage = bCriticalHit ? 2 * Damage + SourceCriticalHitDamage : Damage;


	const FGameplayModifierEvaluatedData EvaluatedData(URPGAttributeSet::GetIncomingDamageAttribute(), EGameplayModOp::Additive, Damage);

	// Adding Evaluated data to output
	OutExecutionOutput.AddOutputModifier(EvaluatedData);
}
