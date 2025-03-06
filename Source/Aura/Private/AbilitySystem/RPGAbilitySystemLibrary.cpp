// Copyright Kyle Murray
#include "AbilitySystem/RPGAbilitySystemLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "UI/HUD/RPGHUD.h"
#include "Player/RPGPlayerState.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "AbilitySystem/RPGAttributeSet.h"
#include "UI/WidgetController/RPGWidgetController.h"
#include "Game/RPGGameModeBase.h"
#include "RPGAbilityTypes.h"
#include "RPGGameplayTags.h"
#include "Interaction/CombatInterface.h"
#include "Game/LoadScreenSaveGame.h"

/// <summary>
/// Creates MakeWidgetControllerParams from given AS,ASC,PC,PS
/// </summary>
/// <param name="WorldContextObject"></param>
/// <returns></returns>
bool URPGAbilitySystemLibrary::MakeWidgetControllerParams(const UObject* WorldContextObject, FWidgetControllerParams& OutWCParams, ARPGHUD*& OutRPGHUD)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		OutRPGHUD = Cast<ARPGHUD>(PC->GetHUD());
		if (OutRPGHUD)
		{
			ARPGPlayerState* PS = PC->GetPlayerState<ARPGPlayerState>();
			UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
			UAttributeSet* AS = PS->GetAttributeSet();

			OutWCParams.AttributeSet = AS;
			OutWCParams.PlayerController = PC;
			OutWCParams.AbilitySystemComponent = ASC;
			OutWCParams.PlayerState = PS;
			
			return true;
		}
	}
	return false;
}

/// <summary>
/// Sets Initial Values for Attributes based off character class
/// </summary>
/// <param name="WorldContextObject"></param>
/// <param name="CharacterClass"></param>
/// <param name="Level"></param>
/// <param name="ASC"></param>
void URPGAbilitySystemLibrary::InitializeDefaultAttributes(const UObject* WorldContextObject, ECharacterClass CharacterClass, float Level, UAbilitySystemComponent* ASC)
{
	// Set the avatar actor from the ASC
	AActor* AvatarActor = ASC->GetAvatarActor();

	// Assign CharacterClassInfo from the GameMode
	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);

	// Assign ClassDefaultInfo
	FCharacterClassDefaultInfo ClassDefaultInfo = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);

	/*
	* Handling Creating and Apply Gameplay Effect for Primary Attributes ------------------------->
	*/

	// Creates effect context for Primary Attributes
	FGameplayEffectContextHandle PrimaryAttributesContexHandle = ASC->MakeEffectContext();
	PrimaryAttributesContexHandle.AddSourceObject(AvatarActor);

	// Spec Handle for PrimaryAttributes
	FGameplayEffectSpecHandle PrimaryAttributesSpecHandle = ASC->MakeOutgoingSpec(ClassDefaultInfo.PrimaryAttributes, Level, PrimaryAttributesContexHandle);

	// Apply PrimaryAttributes to self
	ASC->ApplyGameplayEffectSpecToSelf(*PrimaryAttributesSpecHandle.Data.Get());
	/*
	* Handling Creating and Apply Gameplay Effect for Primary Attributes <-------------------------
	*/


	/*
	* Handling Creating and Apply Gameplay Effect for Secondary Attributes ------------------------->
	*/
	FGameplayEffectContextHandle SecondaryAttributesContexHandle = ASC->MakeEffectContext();
	SecondaryAttributesContexHandle.AddSourceObject(AvatarActor);

	// Spec Handle for SecondaryAttributes
	FGameplayEffectSpecHandle SecondaryAttributesSpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->SecondaryAttributes, Level, SecondaryAttributesContexHandle);

	// Apply SecondaryAttributes to self
	ASC->ApplyGameplayEffectSpecToSelf(*SecondaryAttributesSpecHandle.Data.Get());
	/*
	* Handling Creating and Apply Gameplay Effect for Secondary Attributes <-------------------------
	*/

	/*
	* Handling Creating and Apply Gameplay Effect for Vital Attributes ------------------------->
	*/
	FGameplayEffectContextHandle VitalAttributesContexHandle = ASC->MakeEffectContext();
	VitalAttributesContexHandle.AddSourceObject(AvatarActor);

	// Spec Handle for VitalAttributes
	FGameplayEffectSpecHandle VitalAttributesSpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->VitalAttributes, Level, VitalAttributesContexHandle);

	// Apply VitalAttributes to self
	ASC->ApplyGameplayEffectSpecToSelf(*VitalAttributesSpecHandle.Data.Get());
	/*
	* Handling Creating and Apply Gameplay Effect for Vital Attributes <-------------------------
	*/
}

/// <summary>
/// Sets the saved initial values of Attributes
/// </summary>
/// <param name="WorldContextObject"></param>
/// <param name="ASC"></param>
/// <param name="SaveGame"></param>
void URPGAbilitySystemLibrary::InitializeDefaultAttributesFromSaveData(const UObject* WorldContextObject, UAbilitySystemComponent* ASC, ULoadScreenSaveGame* SaveGame)
{
	// Assign CharacterClassInfo from the GameMode
	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);

	if (!CharacterClassInfo) return;

	const FRPGGameplayTags GameplayTags = FRPGGameplayTags::Get();
	const AActor* SourceAvatarActor = ASC->GetAvatarActor();

	FGameplayEffectContextHandle EffectContextHandle = ASC->MakeEffectContext();
	EffectContextHandle.AddSourceObject(SourceAvatarActor);

	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->PrimaryAttributes_SetByCaller, 1, EffectContextHandle);

	// Sets the Primary Attributes SetByCallerMagnitude
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Attributes_Primary_Intellegence, SaveGame->IntellegenceAttribute);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Attributes_Primary_Strength, SaveGame->StrengthAttribute);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Attributes_Primary_Resilience, SaveGame->ResilienceAttribute);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Attributes_Primary_Vigor, SaveGame->VigorAttribute);

	ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);

	// Secondary Attributes
	FGameplayEffectContextHandle SecondaryAttributesContexHandle = ASC->MakeEffectContext();
	SecondaryAttributesContexHandle.AddSourceObject(SourceAvatarActor);

	// Spec Handle for SecondaryAttributes
	FGameplayEffectSpecHandle SecondaryAttributesSpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->SecondaryAttributes_Infinite, 1, SecondaryAttributesContexHandle);

	// Apply SecondaryAttributes to self
	ASC->ApplyGameplayEffectSpecToSelf(*SecondaryAttributesSpecHandle.Data.Get());

	// Vital Attributes
	FGameplayEffectContextHandle VitalAttributesContexHandle = ASC->MakeEffectContext();
	VitalAttributesContexHandle.AddSourceObject(SourceAvatarActor);

	// Spec Handle for VitalAttributes
	FGameplayEffectSpecHandle VitalAttributesSpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->VitalAttributes, 1, VitalAttributesContexHandle);

	// Apply VitalAttributes to self
	ASC->ApplyGameplayEffectSpecToSelf(*VitalAttributesSpecHandle.Data.Get());
}

/// <summary>
/// Iterates through Arrays of abilities to give on startup and gives them to the ASC
/// </summary>
/// <param name="WorldContextObject"></param>
/// <param name="ASC"></param>
/// <param name="CharacterClass"></param>
void URPGAbilitySystemLibrary::GiveStartupAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* ASC, ECharacterClass CharacterClass)
{
	// Assign CharacterClassInfo from the GameMode
	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	if (!CharacterClassInfo) return;

	// Loops through CommonAbilities
	for (TSubclassOf<UGameplayAbility> AbilityClass : CharacterClassInfo->CommonAbilities)
	{
		//Creates Spec from AbilityClass and Level
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);

		// Give the Ability to the ASC
		ASC->GiveAbility(AbilitySpec);
	}

	const FCharacterClassDefaultInfo DefaultInfo = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);

	// Loops through StartupAbilities
	for (TSubclassOf<UGameplayAbility> AbilityClass : DefaultInfo.StartupAbilities)
	{
		if (ASC->GetAvatarActor()->Implements<UCombatInterface>())
		{
			//Creates Spec from AbilityClass and Level
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, ICombatInterface::Execute_GetPlayerLevel(ASC->GetAvatarActor()));

			// Give the Ability to the ASC
			ASC->GiveAbility(AbilitySpec);
		}
	}
}

/// <summary>
/// Returns whether or not an actor is a friend
/// </summary>
/// <param name="FirstActor"></param>
/// <param name="SecondActor"></param>
/// <returns></returns>
bool URPGAbilitySystemLibrary::IsNotFriend(AActor* FirstActor, AActor* SecondActor)
{
	// Both Actors have player tag
	const bool bBothArePlayers = FirstActor->ActorHasTag(FName("Player")) && SecondActor->ActorHasTag(FName("Player"));

	// Both Actors have enemy tag
	const bool bBothAreEnemies = FirstActor->ActorHasTag(FName("Enemy")) && SecondActor->ActorHasTag(FName("Enemy"));

	const bool bFriends = bBothArePlayers || bBothAreEnemies;

	return !bFriends;
}

/// <summary>
/// Returns the magnitude tags for SetByCaller Magitude for gameplay effects
/// </summary>
/// <param name="GameplayEffec"></param>
/// <returns></returns>
TArray<FGameplayTag> URPGAbilitySystemLibrary::CallerMagnitudeTags(TSubclassOf<UGameplayEffect> GameplayEffect)
{
	// Create new GameplayEffect
	UGameplayEffect* GE = NewObject<UGameplayEffect>(GetTransientPackage(), GameplayEffect);

	// Create new GameplayModifier
	TArray<FGameplayModifierInfo> ModifierInfo = GE->Modifiers;

	TArray<FGameplayTag> CallerTags;

	// Loops through ModifierInfo
	for (FGameplayModifierInfo Info : ModifierInfo)
	{
		// if Info has the type SetByCaller
		if (Info.ModifierMagnitude.GetMagnitudeCalculationType() == EGameplayEffectMagnitudeCalculation::SetByCaller)
		{
			// Add the DataTag to the CallerTags array
			CallerTags.Add(Info.ModifierMagnitude.GetSetByCallerFloat().DataTag);
		}
	}

	return CallerTags;
}

/// <summary>
/// Applies a DamageEffect based off Given Params
/// </summary>
/// <param name="DamageEffectParams"></param>
/// <returns></returns>
FGameplayEffectContextHandle URPGAbilitySystemLibrary::ApplyDamageEffect(const FDamageEffectParams& DamageEffectParams)
{
	const FRPGGameplayTags GameplayTags = FRPGGameplayTags::Get();
	const AActor* SourceAvatarActor = DamageEffectParams.SourceAbilitySystemComponent->GetAvatarActor();

	// Creates EffectContextHandle
	FGameplayEffectContextHandle EffectContextHandle = DamageEffectParams.SourceAbilitySystemComponent->MakeEffectContext();
	EffectContextHandle.AddSourceObject(SourceAvatarActor);

	// Sets the DeathImpulse from DamageEffectParams
	SetDeathImpulse(EffectContextHandle, DamageEffectParams.DeathImpulse);

	// Sets Whether the damage is radial from DamageEffectParams
	SetIsRadialDamage(EffectContextHandle, DamageEffectParams.bIsRadialDamage);
	
	// Dont need to worry about checking if bIsRadialDamage, Check happends in Effect Context
	SetRadialDamageInnerRadius(EffectContextHandle, DamageEffectParams.RadialDamageInnerRadius);
	SetRadialDamageOuterRadius(EffectContextHandle, DamageEffectParams.RadialDamageOuterRadius);
	SetRadialDamageOrigin(EffectContextHandle, DamageEffectParams.RadialDamageOrigin);

	// Creates SpecHandle
	FGameplayEffectSpecHandle SpecHandle = DamageEffectParams.SourceAbilitySystemComponent->MakeOutgoingSpec(DamageEffectParams.DamageGameplayEffectClass, DamageEffectParams.AbilityLevel, EffectContextHandle);

	// Sets the Damage SetByCallerMagnitude
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, DamageEffectParams.DamageType, DamageEffectParams.BaseDamage);

	// Sets the Debuffs SetByCallerMagnitude
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Chance, DamageEffectParams.DebuffChance);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Damage, DamageEffectParams.DebuffDamage);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Frequency, DamageEffectParams.DebuffFrequency);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Duration, DamageEffectParams.DebuffDuration);

	// Applies the GA Spec to self
	DamageEffectParams.TargetAbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);

	return EffectContextHandle;
}

/// <summary>
/// Calculates the spread of rotators (Used for projectiles)
/// </summary>
/// <param name="Forward"></param>
/// <param name="Axis"></param>
/// <param name="Spread"></param>
/// <param name="NumRotators"></param>
/// <returns></returns>
TArray<FRotator> URPGAbilitySystemLibrary::EvenlySpacedRotators(const FVector& Forward, const FVector& Axis, float Spread, int32 NumRotators)
{
	TArray<FRotator> Rotators;
	const FVector LeftOfSpread = Forward.RotateAngleAxis(-Spread * 0.5, Axis);

	if (NumRotators > 1)
	{
		// Spread Of Projectiles
		const float DeltaSpread = Spread / (NumRotators - 1);

		for (int32 i = 0; i < NumRotators; i++)
		{
			const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread * i, Axis);
			Rotators.Add(Direction.Rotation());
		}
	}

	else
	{
		Rotators.Add(Forward.Rotation());
	}

	return Rotators;
}

/// <summary>
/// Calculates the spread of Vectors (Used for projectiles)
/// </summary>
/// <param name="Forward"></param>
/// <param name="Axis"></param>
/// <param name="Spread"></param>
/// <param name="NumVectors"></param>
/// <returns></returns>
TArray<FVector> URPGAbilitySystemLibrary::EvenlyRotatedVectors(const FVector& Forward, const FVector& Axis, float Spread, int32 NumVectors)
{
	TArray<FVector> Vectors;
	const FVector LeftOfSpread = Forward.RotateAngleAxis(-Spread * 0.5, Axis);

	if (NumVectors > 1)
	{
		// Spread Of Projectiles
		const float DeltaSpread = Spread / (NumVectors - 1);

		for (int32 i = 0; i < NumVectors; i++)
		{
			const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread * i, Axis);
			Vectors.Add(Direction);
		}
	}

	else
	{
		Vectors.Add(Forward);
	}

	return Vectors;
}

/// <summary>
/// Calculates radial damage based off distance from origin
/// </summary>
/// <param name="EffectContextHandle"></param>
/// <param name="Damage"></param>
/// <param name="TargetAvatar"></param>
/// <returns></returns>
float URPGAbilitySystemLibrary::CalculateRadialDamage(const FGameplayEffectContextHandle& EffectContextHandle, const float Damage, const AActor* TargetAvatar)
{
	// Get the location of target
	FVector TargetLocation = TargetAvatar->GetActorLocation();

	// Get the origin of target
	const FVector Origin = URPGAbilitySystemLibrary::GetRadialDamageOrigin(EffectContextHandle);

	TargetLocation.Z = Origin.Z; // TargetAvatar halfheight may be above the InnerRadius
	
	// Calculate Sqr distance between Target location and origin
	const float SquareDistance = FVector::DistSquared(TargetLocation, Origin);

	// Get Sqr Inner Radius
	const float SquareInnerRadius = FMath::Square(URPGAbilitySystemLibrary::GetRadialDamageInnerRadius(EffectContextHandle));

	// Get Sqr Outer Radius
	const float SquareOuterRadius = FMath::Square(URPGAbilitySystemLibrary::GetRadialDamageOuterRadius(EffectContextHandle));

	if (SquareDistance <= SquareInnerRadius) return Damage;

	// Calculate Damage 
	const TRange<float> DistanceRange(SquareInnerRadius, SquareOuterRadius);
	const TRange<float> DamageScaleRange(1.0f, 0.f);
	const float DamageScale = FMath::GetMappedRangeValueClamped(DistanceRange, DamageScaleRange, SquareDistance);

	const float RadialDamage = Damage * DamageScale;

	return RadialDamage;
}

/// <summary>
/// Returns the LootTiers from the GameMode
/// </summary>
/// <param name="WorldContextObject"></param>
/// <returns></returns>
ULootTiers* URPGAbilitySystemLibrary::GetLootTiers(const UObject* WorldContextObject)
{
	ARPGGameModeBase* RPGGameMode = Cast<ARPGGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (!RPGGameMode) return nullptr;

	return RPGGameMode->LootTiers;
}

/*Setters ------------------------------------------------------------->*/

void URPGAbilitySystemLibrary::SetIsBlockedHit(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, bool bInIsBlockedHit)
{
	if (FRPGGameplayEffectContext* RPGContext = static_cast<FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return RPGContext->SetIsBlockedlHit(bInIsBlockedHit);
	}
}

void URPGAbilitySystemLibrary::SetIsCriticalHit(UPARAM(ref)FGameplayEffectContextHandle& EffectContextHandle, bool bInIsCriticalHit)
{
	if (FRPGGameplayEffectContext* RPGContext = static_cast<FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return RPGContext->SetIsCriticalHit(bInIsCriticalHit);
	}
}

void URPGAbilitySystemLibrary::SetIsSuccessfulDebuff(UPARAM(ref)FGameplayEffectContextHandle& EffectContextHandle, bool bInIsSuccessfulDebuff)
{
	if (FRPGGameplayEffectContext* RPGContext = static_cast<FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return RPGContext->SetIsSuccessfulDebuff(bInIsSuccessfulDebuff);
	}
}

void URPGAbilitySystemLibrary::SetDebuffDamage(UPARAM(ref)FGameplayEffectContextHandle& EffectContextHandle, float InDebuffDamage)
{
	if (FRPGGameplayEffectContext* RPGContext = static_cast<FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return RPGContext->SetDebuffDamage(InDebuffDamage);
	}
}

void URPGAbilitySystemLibrary::SetDebuffDuration(UPARAM(ref)FGameplayEffectContextHandle& EffectContextHandle, float InDebuffDuration)
{
	if (FRPGGameplayEffectContext* RPGContext = static_cast<FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return RPGContext->SetDebuffDuration(InDebuffDuration);
	}
}

void URPGAbilitySystemLibrary::SetDebuffFrequency(UPARAM(ref)FGameplayEffectContextHandle& EffectContextHandle, float InDebuffFrequency)
{
	if (FRPGGameplayEffectContext* RPGContext = static_cast<FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return RPGContext->SetDebuffFrequency(InDebuffFrequency);
	}
}

void URPGAbilitySystemLibrary::SetDamageType(UPARAM(ref)FGameplayEffectContextHandle& EffectContextHandle, FGameplayTag& InDamageType)
{
	if (FRPGGameplayEffectContext* RPGContext = static_cast<FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		TSharedPtr<FGameplayTag> DamageType = MakeShared<FGameplayTag>(InDamageType);
		return RPGContext->SetDamageType(DamageType);
	}
}

void URPGAbilitySystemLibrary::SetDeathImpulse(UPARAM(ref)FGameplayEffectContextHandle& EffectContextHandle, FVector InDeathImpulse)
{
	if (FRPGGameplayEffectContext* RPGContext = static_cast<FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return RPGContext->SetDeathImpulse(InDeathImpulse);
	}
}

void URPGAbilitySystemLibrary::SetKnockbackForce(UPARAM(ref)FGameplayEffectContextHandle& EffectContextHandle, FVector InKnockbackForce)
{
	if (FRPGGameplayEffectContext* RPGContext = static_cast<FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return RPGContext->SetDeathImpulse(InKnockbackForce);
	}
}

void URPGAbilitySystemLibrary::SetIsRadialDamage(UPARAM(ref)FGameplayEffectContextHandle& EffectContextHandle, bool bInIsRadialDamage)
{
	if (FRPGGameplayEffectContext* RPGContext = static_cast<FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return RPGContext->SetIsRadialDamage(bInIsRadialDamage);
	}
}

void URPGAbilitySystemLibrary::SetRadialDamageInnerRadius(UPARAM(ref)FGameplayEffectContextHandle& EffectContextHandle, float InRadialDamageInnerRadius)
{
	if (FRPGGameplayEffectContext* RPGContext = static_cast<FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return RPGContext->SetRadialDamageInnerRadius(InRadialDamageInnerRadius);
	}
}

void URPGAbilitySystemLibrary::SetRadialDamageOuterRadius(UPARAM(ref)FGameplayEffectContextHandle& EffectContextHandle, float InRadialDamageOuterRadius)
{
	if (FRPGGameplayEffectContext* RPGContext = static_cast<FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return RPGContext->SetRadialDamageOuterRadius(InRadialDamageOuterRadius);
	}
}

void URPGAbilitySystemLibrary::SetRadialDamageOrigin(UPARAM(ref)FGameplayEffectContextHandle& EffectContextHandle, FVector InRadialDamageOrigin)
{
	if (FRPGGameplayEffectContext* RPGContext = static_cast<FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return RPGContext->SetRadialDamageOrigin(InRadialDamageOrigin);
	}
}

/*Getters ------------------------------------------------------------->*/

/// <summary>
/// Returns the OverlayWidgetController
/// </summary>
/// <param name="WorldContextObject"></param>
/// <returns></returns>
UOverlayWidgetController* URPGAbilitySystemLibrary::GetOverlayWidgetController(const UObject* WorldContextObject)
{
	FWidgetControllerParams WCParams;
	ARPGHUD* RPGHUD = nullptr;

	if (MakeWidgetControllerParams(WorldContextObject, WCParams, RPGHUD))
	{
		return RPGHUD->GetOverlayWidgetController(WCParams);
	}

	return nullptr;
}

/// <summary>
/// Returns the AttributeMenuWidgetController
/// </summary>
/// <param name="WorldContextObject"></param>
/// <returns></returns>
UAttributeMenuWidgetController* URPGAbilitySystemLibrary::GetAttributeMenuWidgetController(const UObject* WorldContextObject)
{
	FWidgetControllerParams WCParams;
	ARPGHUD* RPGHUD = nullptr;

	if (MakeWidgetControllerParams(WorldContextObject, WCParams, RPGHUD))
	{
		return RPGHUD->GetAttributeMenuWidgetController(WCParams);
	}

	return nullptr;
}

/// <summary>
/// Returns the SpellMenuWidgetController
/// </summary>
/// <param name="WorldContextObject"></param>
/// <returns></returns>
USpellMenuWidgetController* URPGAbilitySystemLibrary::GetSpellMenuWidgetController(const UObject* WorldContextObject)
{
	FWidgetControllerParams WCParams;
	ARPGHUD* RPGHUD = nullptr;

	if (MakeWidgetControllerParams(WorldContextObject, WCParams, RPGHUD))
	{
		return RPGHUD->GetSpellMenuWidgetController(WCParams);
	}
	return nullptr;
}

/// <summary>
/// Returns the amount of XP to be rewarded for killing an enemy based on class and level
/// </summary>
/// <param name="WorldContextObject"></param>
/// <param name="CharacterClass"></param>
/// <param name="CharacterLevel"></param>
/// <returns></returns>
int32 URPGAbilitySystemLibrary::GetXPRewardForClassAndLevel(const UObject* WorldContextObject, ECharacterClass CharacterClass, int32 CharacterLevel)
{
	// Assign CharacterClassInfo from the GameMode
	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	if (!CharacterClassInfo) return 0;

	const FCharacterClassDefaultInfo& Info = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);

	float XPReward = Info.XPReward.GetValueAtLevel(CharacterLevel);

	return static_cast<int32>(XPReward);
}

/// <summary>
/// Returns all Alive players within a given radius
/// </summary>
/// <param name="WorldContextObject"></param>
/// <param name="OutOverlappingActors"></param>
/// <param name="ActorsToIgnore"></param>
/// <param name="Radius"></param>
/// <param name="SphereOrigin"></param>
void URPGAbilitySystemLibrary::GetLivePlayersWithinRadius(const UObject* WorldContextObject, TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& ActorsToIgnore, float Radius, const FVector SphereOrigin)
{
	FCollisionQueryParams SphereParams;

	SphereParams.AddIgnoredActors(ActorsToIgnore);

	TArray<FOverlapResult> Overlaps;

	if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		World->OverlapMultiByObjectType(Overlaps, SphereOrigin, FQuat::Identity, FCollisionObjectQueryParams(FCollisionObjectQueryParams::InitType::AllDynamicObjects), FCollisionShape::MakeSphere(Radius), SphereParams);

		for (FOverlapResult& Overlap : Overlaps)
		{
			if (Overlap.GetActor()->Implements<UCombatInterface>() && !ICombatInterface::Execute_IsDead(Overlap.GetActor()))
			{
				OutOverlappingActors.AddUnique(ICombatInterface::Execute_GetAvatar(Overlap.GetActor()));
			}
		}
	}
}

/// <summary>
/// Return the closest targets from an array of actors
/// </summary>
/// <param name="MaxTargets"></param>
/// <param name="Actors"></param>
/// <param name="OutClosestTargets"></param>
/// <param name="Origin"></param>
void URPGAbilitySystemLibrary::GetClosestTargets(int32 MaxTargets, const TArray<AActor*>& Actors, TArray<AActor*>& OutClosestTargets, const FVector& Origin)
{
	//Check if MaxTargets or Actors and return
	if (MaxTargets == 0 || Actors.IsEmpty()) return;

	// Local array for sorted actors
	TArray<AActor*> SortedActors = Actors;

	// Check if sorted actors is leass than MaxTargets
	if (SortedActors.Num() <= MaxTargets)
	{
		OutClosestTargets = SortedActors;
	}

	// Sort the actirs based of thier distance from the origin
	Algo::Sort(SortedActors, [Origin](const AActor* ActorA, const AActor* ActorB)
		{
			const float DistanceA = FVector::DistSquared(ActorA->GetActorLocation(), Origin);
			const float DistanceB = FVector::DistSquared(ActorB->GetActorLocation(), Origin);
			return DistanceA < DistanceB;
		});

	// Return the first elements of SortedActors up to max targets
	int32 Size = FMath::Min(MaxTargets, SortedActors.Num());
	for (int32 i = 0; i < Size; i++)
	{
		OutClosestTargets.Add(SortedActors[i]);
	}
}

/// <summary>
/// Return the CharacterClassInfo from the GameMode
/// </summary>
/// <param name="WorldContextObject"></param>
/// <returns></returns>
UCharacterClassInfo* URPGAbilitySystemLibrary::GetCharacterClassInfo(const UObject* WorldContextObject)
{
	ARPGGameModeBase* RPGGameMode = Cast<ARPGGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (!RPGGameMode) return nullptr;

	return RPGGameMode->CharacterClassInfo;
}

/// <summary>
/// Return the AbilityInfo from the GameMode
/// </summary>
/// <param name="WorldContextObject"></param>
/// <returns></returns>
UAbilityInfo* URPGAbilitySystemLibrary::GetAbilityInfo(const UObject* WorldContextObject)
{
	ARPGGameModeBase* RPGGameMode = Cast<ARPGGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (!RPGGameMode) return nullptr;

	return RPGGameMode->AbilityInfo;
}

/// <summary>
/// Returns whether or not the hit was blocked from RPGContext
/// </summary>
/// <param name="EffectContextHandle"></param>
/// <returns></returns>
bool URPGAbilitySystemLibrary::IsBlockedHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FRPGGameplayEffectContext* RPGContext = static_cast<const FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return RPGContext->IsBlockedHit();
	}

	return false;
}

/// <summary>
/// Returns whether or not the hit was a crit from RPGContext
/// </summary>
/// <param name="EffectContextHandle"></param>
/// <returns></returns>
bool URPGAbilitySystemLibrary::IsCriticalHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FRPGGameplayEffectContext* RPGContext = static_cast<const FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return RPGContext->IsCriticalHit();
	}

	return false;
}

/// <summary>
/// Returns whether or not the hit had a successful debuff from RPGContext
/// </summary>
/// <param name="EffectContextHandle"></param>
/// <returns></returns>
bool URPGAbilitySystemLibrary::IsSuccessfulDebuff(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FRPGGameplayEffectContext* RPGContext = static_cast<const FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return RPGContext->IsSuccessfulDebuff();
	}
	return false;
}

/// <summary>
/// Return the Debuff Damage from RPGContext
/// </summary>
/// <param name="EffectContextHandle"></param>
/// <returns></returns>
float URPGAbilitySystemLibrary::GetDebuffDamage(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FRPGGameplayEffectContext* RPGContext = static_cast<const FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return RPGContext->GetDebuffDamage();
	}

	return 0.0f;
}

/// <summary>
/// Return the Debuff Duration from RPGContext
/// </summary>
/// <param name="EffectContextHandle"></param>
/// <returns></returns>
float URPGAbilitySystemLibrary::GetDebuffDuration(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FRPGGameplayEffectContext* RPGContext = static_cast<const FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return RPGContext->GetDebuffDuration();
	}

	return 0.0f;
}

/// <summary>
/// Return the Debuff Frequency from RPGContext
/// </summary>
/// <param name="EffectContextHandle"></param>
/// <returns></returns>
float URPGAbilitySystemLibrary::GetDebuffFrequency(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FRPGGameplayEffectContext* RPGContext = static_cast<const FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return RPGContext->GetDebuffFrequency();
	}
	return 0.0f;
}

/// <summary>
/// Return the Damage type from RPGContext
/// </summary>
/// <param name="EffectContextHandle"></param>
/// <returns></returns>
FGameplayTag URPGAbilitySystemLibrary::GetDamageType(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FRPGGameplayEffectContext* RPGContext = static_cast<const FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		if (RPGContext->GetDamageType().IsValid())
		{
			return *RPGContext->GetDamageType();
		}
	}
	return FGameplayTag();
}

/// <summary>
/// Return the Death Impulse from RPGContext
/// </summary>
/// <param name="EffectContextHandle"></param>
/// <returns></returns>
FVector URPGAbilitySystemLibrary::GetDealthImpulse(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FRPGGameplayEffectContext* RPGContext = static_cast<const FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return RPGContext->GetDeathImpulse();
	}

	return FVector::ZeroVector;
}

/// <summary>
/// Return the KnockbackForce from RPGContext
/// </summary>
/// <param name="EffectContextHandle"></param>
/// <returns></returns>
FVector URPGAbilitySystemLibrary::GetKnockbackForce(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FRPGGameplayEffectContext* RPGContext = static_cast<const FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return RPGContext->GetKnockbackForce();
	}

	return FVector::ZeroVector;
}

/// <summary>
/// Return whether or not the damage is radial from RPGContext
/// </summary>
/// <param name="EffectContextHandle"></param>
/// <returns></returns>
bool URPGAbilitySystemLibrary::IsRadialDamage(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FRPGGameplayEffectContext* RPGContext = static_cast<const FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return RPGContext->IsRadialDamage();
	}
	return false;
}


/// <summary>
/// Return the RadialDamageInnerRadius from RPGContext
/// </summary>
/// <param name="EffectContextHandle"></param>
/// <returns></returns>
float URPGAbilitySystemLibrary::GetRadialDamageInnerRadius(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FRPGGameplayEffectContext* RPGContext = static_cast<const FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return RPGContext->GetRadialDamageInnerRadius();
	}
	return 0.0f;
}

/// <summary>
/// Return the RadialDamageOuterRadius from RPGContext
/// </summary>
/// <param name="EffectContextHandle"></param>
/// <returns></returns>
float URPGAbilitySystemLibrary::GetRadialDamageOuterRadius(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FRPGGameplayEffectContext* RPGContext = static_cast<const FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return RPGContext->GetRadialDamageOuterRadius();
	}
	return 0.0f;
}

/// <summary>
/// Return the RadialDamageOrigin from RPGContext
/// </summary>
/// <param name="EffectContextHandle"></param>
/// <returns></returns>
FVector URPGAbilitySystemLibrary::GetRadialDamageOrigin(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FRPGGameplayEffectContext* RPGContext = static_cast<const FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return RPGContext->GetRadialDamageOrigin();
	}

	return FVector::ZeroVector;
}
