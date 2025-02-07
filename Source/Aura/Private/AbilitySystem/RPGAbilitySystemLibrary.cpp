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

/// <summary>
/// 
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
/// 
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
/// 
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
/// 
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
/// 
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
/// 
/// </summary>
/// <param name="WorldContextObject"></param>
/// <param name="ASC"></param>
/// <param name="CharacterClass"></param>
void URPGAbilitySystemLibrary::GiveStartupAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* ASC, ECharacterClass CharacterClass)
{
	// Assign CharacterClassInfo from the GameMode
	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	if (!CharacterClassInfo) return;

	for (TSubclassOf<UGameplayAbility> AbilityClass : CharacterClassInfo->CommonAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);

		ASC->GiveAbility(AbilitySpec);
	}

	const FCharacterClassDefaultInfo DefaultInfo = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);

	for (TSubclassOf<UGameplayAbility> AbilityClass : DefaultInfo.StartupAbilities)
	{
		if (ASC->GetAvatarActor()->Implements<UCombatInterface>())
		{
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, ICombatInterface::Execute_GetPlayerLevel(ASC->GetAvatarActor()));

			ASC->GiveAbility(AbilitySpec);
		}
	}
}

/// <summary>
/// 
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
/// 
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
/// 
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
/// 
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
/// 
/// </summary>
/// <param name="EffectContextHandle, Passed in as a UPARAM(ref) to make it an input parameter"></param>
/// <param name="bInIsBlockedHit"></param>
void URPGAbilitySystemLibrary::SetIsBlockedHit(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, bool bInIsBlockedHit)
{
	if (FRPGGameplayEffectContext* RPGContext = static_cast< FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return RPGContext->SetIsBlockedlHit(bInIsBlockedHit);
	}
}

/// <summary>
/// 
/// </summary>
/// <param name="EffectContextHandle, Passed in as a UPARAM(ref) to make it an input parameter "></param> 
/// <param name="bInIsCriticalHit"></param>
void URPGAbilitySystemLibrary::SetIsCriticalHit(UPARAM(ref)FGameplayEffectContextHandle& EffectContextHandle, bool bInIsCriticalHit)
{
	if (FRPGGameplayEffectContext* RPGContext = static_cast<FRPGGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return RPGContext->SetIsCriticalHit(bInIsCriticalHit);
	}
}

/// <summary>
/// 
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
	UGameplayEffect* GE = NewObject<UGameplayEffect>(GetTransientPackage(), GameplayEffect);

	TArray<FGameplayModifierInfo> ModifierInfo = GE->Modifiers;

	TArray<FGameplayTag> CallerTags;

	for (FGameplayModifierInfo Info : ModifierInfo)
	{
		if (Info.ModifierMagnitude.GetMagnitudeCalculationType() == EGameplayEffectMagnitudeCalculation::SetByCaller)
		{
			CallerTags.Add(Info.ModifierMagnitude.GetSetByCallerFloat().DataTag);
		}
	}

	return CallerTags;
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

	float XPReward =  Info.XPReward.GetValueAtLevel(CharacterLevel);

	return static_cast<int32>(XPReward);
}

FGameplayEffectContextHandle URPGAbilitySystemLibrary::ApplyDamageEffect(const FDamageEffectParams& DamageEffectParams)
{
	const FRPGGameplayTags GameplayTags = FRPGGameplayTags::Get();
	const AActor* SourceAvatarActor = DamageEffectParams.SourceAbilitySystemComponent->GetAvatarActor();

	FGameplayEffectContextHandle EffectContextHandle = DamageEffectParams.SourceAbilitySystemComponent->MakeEffectContext();
	EffectContextHandle.AddSourceObject(SourceAvatarActor);

	FGameplayEffectSpecHandle SpecHandle = DamageEffectParams.SourceAbilitySystemComponent->MakeOutgoingSpec(DamageEffectParams.DamageGameplayEffectClass, DamageEffectParams.AbilityLevel, EffectContextHandle);

	// Sets the Damage SetByCallerMagnitude
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, DamageEffectParams.DamageType, DamageEffectParams.BaseDamage);

	// Sets the Debuffs SetByCallerMagnitude
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Chance, DamageEffectParams.DebuffChance);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Damage, DamageEffectParams.DebuffDamage);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Frequency, DamageEffectParams.DebuffFrequency);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Duration, DamageEffectParams.DebuffDuration);

	DamageEffectParams.TargetAbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);

	return EffectContextHandle;
}
