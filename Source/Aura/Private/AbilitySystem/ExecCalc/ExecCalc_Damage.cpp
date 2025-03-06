// Copyright Kyle Murray


#include "AbilitySystem/ExecCalc/ExecCalc_Damage.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/RPGAttributeSet.h"
#include "RPGGameplayTags.h"
#include "Interaction/CombatInterface.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "AbilitySystem/RPGAbilitySystemLibrary.h"
#include "RPGAbilityTypes.h"
#include "GameplayTagContainer.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystem/Abilities/Passive/RPGHaloOfProtection.h"


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
	TMap<FGameplayTag, FGameplayEffectAttributeCaptureDefinition> TagsToCaptureDefs;

	const FRPGGameplayTags& Tags = FRPGGameplayTags::Get();
	TagsToCaptureDefs.Add(Tags.Attributes_Secondary_Armor,DamageStatics().ArmorDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Secondary_BlockChance, DamageStatics().BlockChanceDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Secondary_ArmorPenetration, DamageStatics().ArmorPenetrationDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Secondary_CritcalHitChance, DamageStatics().CriticalHitChanceDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Secondary_CritcalHitDamage, DamageStatics().CriticalHitDamageDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Secondary_CritcalHitResistance, DamageStatics().CriticalHitResistanceDef);

	TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Arcane, DamageStatics().ArcaneResistanceDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Fire, DamageStatics().FireResistanceDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Lightning, DamageStatics().LightningResistanceDef);
	TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Physical, DamageStatics().PhysicalResistanceDef);

	/*Setting up needed variables ------------------------------------------------------------------------------->*/

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
	
	// Gets Target and Source Tags
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	int32 SourcePlayerLevel = 1;
	if (SourceAvatar->Implements<UCombatInterface>())
	{
		SourcePlayerLevel = ICombatInterface::Execute_GetPlayerLevel(SourceAvatar);
	}

	int32 TargetPlayerLevel = 1;
	if (TargetAvatar->Implements<UCombatInterface>())
	{
		TargetPlayerLevel = ICombatInterface::Execute_GetPlayerLevel(TargetAvatar);
	}

	// Getting the CharacterClassInfo from our blueprint system library
	UCharacterClassInfo* CharacterClassInfo = URPGAbilitySystemLibrary::GetCharacterClassInfo(SourceAvatar);

	FGameplayEffectContextHandle EffectContextHandle = Spec.GetContext();
	/*Setting up needed variables end <----------------------------------------------------------------------------*/

	/*Hanlding Debuffs*/
	DetermineDebuff(Spec, ExecutionParams, EvaluationParameters,TagsToCaptureDefs);

	// Get Damage Set By Caller Magnitude
	float Damage = 0;

	/*Hanlding Resistances*/
	HandleResistances(TagsToCaptureDefs, Spec, ExecutionParams, EvaluationParameters, EffectContextHandle, TargetAvatar, Damage);

	/*Capture Block Chance on target and Determine if there was a successful block ->*/ 
	float TargetBlockChance = 0;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().BlockChanceDef, EvaluationParameters, TargetBlockChance);
	TargetBlockChance = FMath::Max<float>(TargetBlockChance,0);

	const bool bBlocked = FMath::RandRange(1, 100) < TargetBlockChance;
	
	URPGAbilitySystemLibrary::SetIsBlockedHit(EffectContextHandle, bBlocked);

	// If Block, halve the damage
	Damage = bBlocked ? Damage *= 0.5 : Damage;
	/*<----------------------------------------------------------------------------------*/

	/*Hanlding Armor*/
	HandleTargetArmor(ExecutionParams, EvaluationParameters, CharacterClassInfo, SourcePlayerLevel, TargetPlayerLevel, Damage);

	/*Handling Critical Damage -------------------------------------------------------------------------------------------------->*/

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
	FRealCurve* CriticalHitResistanceCurve = CharacterClassInfo->DamageCalculationCoefficients->FindCurve(FName("CriticalHitResistance"), FString());

	// Gets the ArmorPenetrationCoefficient from the curve based on player level
	const float CriticalHitResistanceCoefficient = CriticalHitResistanceCurve->Eval(SourcePlayerLevel);

	// Critical hit chance calculation, Critical Hit Resistance reduces Critical Hit Chance by a certain percentage
	const float EffectiveCriticalHitChance = SourceCriticalHitChance - TargetCriticalHitResistance * CriticalHitResistanceCoefficient;

	const bool bCriticalHit = FMath::RandRange(1, 100) < EffectiveCriticalHitChance;

	URPGAbilitySystemLibrary::SetIsCriticalHit(EffectContextHandle, bCriticalHit);

	// If crit hit scale damage by double and add SourceCriticalHitDamage
	Damage = bCriticalHit ? 2 * Damage + SourceCriticalHitDamage : Damage;

	/*Hanlding Critical Damage End <--------------------------------------------------------------------------------------------------*/

	// Makes sure damage is at least 1
	float FinalDamage = FMath::Max(Damage,1);

	const FGameplayModifierEvaluatedData EvaluatedData(URPGAttributeSet::GetIncomingDamageAttribute(), EGameplayModOp::Additive, FinalDamage);

	// Adding Evaluated data to output
	OutExecutionOutput.AddOutputModifier(EvaluatedData);
}

/// <summary>
/// Handles calculating damage based off targets armor
/// </summary>
/// <param name="ExecutionParams"></param>
/// <param name="EvaluationParameters"></param>
/// <param name="CharacterClassInfo"></param>
/// <param name="SourcePlayerLevel"></param>
/// <param name="TargetPlayerLevel"></param>
/// <param name="Damage"></param>
void UExecCalc_Damage::HandleTargetArmor(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FAggregatorEvaluateParameters& EvaluationParameters, UCharacterClassInfo* CharacterClassInfo, int32 SourcePlayerLevel, int32 TargetPlayerLevel, float& Damage) const
{
	float TargetArmor = 0;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorDef, EvaluationParameters, TargetArmor);
	TargetArmor = FMath::Max<float>(TargetArmor, 0);
	// <-

	// Capture the Sources ArmorPenetration ->
	float SourceArmorPenetration = 0;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorPenetrationDef, EvaluationParameters, SourceArmorPenetration);
	SourceArmorPenetration = FMath::Max<float>(SourceArmorPenetration, 0);
	// <-


	// Get ArmorPenetrationCurve from our data asset
	FRealCurve* ArmorPenetrationCurve = CharacterClassInfo->DamageCalculationCoefficients->FindCurve(FName("ArmorPenetration"), FString());

	// Gets the ArmorPenetrationCoefficient from the curve based on player level
	const float ArmorPenetrationCoefficient = ArmorPenetrationCurve->Eval(SourcePlayerLevel);

	// ArmorPenetration ignores a percentage of targets armor
	const float EffectiveArmor = TargetArmor * (100 - SourceArmorPenetration * ArmorPenetrationCoefficient) / 100;

	// Get EffectiveArmorCurve from our data asset
	FRealCurve* EffectiveArmorCurve = CharacterClassInfo->DamageCalculationCoefficients->FindCurve(FName("EffectiveArmor"), FString());

	// Gets the EffectiveArmorCoefficient from the curve based on player level
	const float EffectiveArmorCoefficient = EffectiveArmorCurve->Eval(TargetPlayerLevel);

	// Armor ignores a precentage of incoming damage
	Damage *= (100 - EffectiveArmor * EffectiveArmorCoefficient) / 100;
}

/// <summary>
/// Handles calculating damage based off resistances
/// </summary>
/// <param name="TagsToCaptureDefs"></param>
/// <param name="Spec"></param>
/// <param name="ExecutionParams"></param>
/// <param name="EvaluationParameters"></param>
/// <param name="EffectContextHandle"></param>
/// <param name="TargetAvatar"></param>
/// <param name="Damage"></param>
void UExecCalc_Damage::HandleResistances(TMap<FGameplayTag, FGameplayEffectAttributeCaptureDefinition>& TagsToCaptureDefs, const FGameplayEffectSpec& Spec, const FGameplayEffectCustomExecutionParameters& ExecutionParams, FAggregatorEvaluateParameters& EvaluationParameters, FGameplayEffectContextHandle& EffectContextHandle, AActor* TargetAvatar, float& Damage) const
{
	// Loop through DamageTypesToResistances
	for (const TPair<FGameplayTag, FGameplayTag>& Pair : FRPGGameplayTags::Get().DamageTypesToResistances)
	{
		const FGameplayTag DamageType = Pair.Key;
		const FGameplayTag ResistanceTag = Pair.Value;

		checkf(TagsToCaptureDefs.Contains(ResistanceTag), TEXT("TagsToCaptureDefs doesn't containg tag [%s] in ExecCalc_Damage"), *ResistanceTag.ToString());
		const FGameplayEffectAttributeCaptureDefinition CaptureDef = TagsToCaptureDefs[ResistanceTag];

		float DamageTypeValue = Spec.GetSetByCallerMagnitude(Pair.Key, false, 0);

		float Resistance = 0;
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(CaptureDef, EvaluationParameters, Resistance);

		// Checks for passive HOP Ability
		if (EvaluationParameters.TargetTags->HasTagExact(FRPGGameplayTags::Get().Abilities_Passive_HaloOfProtection))
		{
			// adds 20 to
			Resistance += 20;
		}

		// Clamp resistance to 75
		Resistance = FMath::Clamp(Resistance, 0, 75);

		// Resistance Calculation
		DamageTypeValue *= (100 - Resistance) / 100;

		// Radial Damage
		if (URPGAbilitySystemLibrary::IsRadialDamage(EffectContextHandle))
		{
			DamageTypeValue = URPGAbilitySystemLibrary::CalculateRadialDamage(EffectContextHandle, DamageTypeValue, TargetAvatar);
		}

		// Add calculated damage to damage
		Damage += DamageTypeValue;
	}
}

/// <summary>
/// Handles what happends when a debuff is successful
/// </summary>
/// <param name="Spec"></param>
/// <param name="ExecutionParams"></param>
/// <param name="EvaluationParameters"></param>
void UExecCalc_Damage::DetermineDebuff(const FGameplayEffectSpec& Spec, const FGameplayEffectCustomExecutionParameters& ExecutionParams, FAggregatorEvaluateParameters& EvaluationParameters, const TMap<FGameplayTag, FGameplayEffectAttributeCaptureDefinition>& InTagsToDefs) const
{
	const FRPGGameplayTags& GameplayTags = FRPGGameplayTags::Get();

	// Looping over debuff types
	for (TTuple<FGameplayTag, FGameplayTag> Pair : GameplayTags.DamageTypesToDebuffs)
	{
		FGameplayTag& DamageType = Pair.Key;
		FGameplayTag& DebuffType = Pair.Value;

		float TypeDamage = Spec.GetSetByCallerMagnitude(DamageType, false, -1);

		if (TypeDamage > -1)
		{
			// Determine if there was a successful debuff
			const float SourceDebuffChance = Spec.GetSetByCallerMagnitude(GameplayTags.Debuff_Chance, false, -1);

			float TargetDebuffResistance = 0;
			const FGameplayTag& ResistanceTag = GameplayTags.DamageTypesToResistances[DamageType];
			ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(InTagsToDefs[ResistanceTag], EvaluationParameters, TargetDebuffResistance);

			TargetDebuffResistance = FMath::Max<float>(TargetDebuffResistance, 0);

			// Calculate Debuff chance
			const float EffectiveDebuffChance = SourceDebuffChance * (100 - TargetDebuffResistance) / 100;

			// Roll the dice to see if bDebuff is true
			const bool bDebuff = FMath::RandRange(1, 100) < EffectiveDebuffChance;

			// If debuff was successful
			if (bDebuff)
			{
				// Get the context handle
				FGameplayEffectContextHandle ContextHandle = Spec.GetContext();

				// Set Damage Type and SuccessfulDebuff
				URPGAbilitySystemLibrary::SetIsSuccessfulDebuff(ContextHandle,true);
				URPGAbilitySystemLibrary::SetDamageType(ContextHandle, DamageType);

				// Get Debuff info from Spec
				const float DebuffDamage = Spec.GetSetByCallerMagnitude(GameplayTags.Debuff_Damage, false, -1);
				const float DebuffDuration = Spec.GetSetByCallerMagnitude(GameplayTags.Debuff_Duration, false, -1);
				const float DebuffFrequency = Spec.GetSetByCallerMagnitude(GameplayTags.Debuff_Frequency, false, -1);

				// Set Debuff Info
				URPGAbilitySystemLibrary::SetDebuffDamage(ContextHandle, DebuffDamage);
				URPGAbilitySystemLibrary::SetDebuffDuration(ContextHandle, DebuffDuration);
				URPGAbilitySystemLibrary::SetDebuffFrequency(ContextHandle, DebuffFrequency);
			}
		}
	}
}
