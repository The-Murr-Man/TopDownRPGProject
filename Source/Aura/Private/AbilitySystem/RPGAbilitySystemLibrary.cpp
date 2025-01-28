// Copyright Kyle Murray


#include "AbilitySystem/RPGAbilitySystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "UI/HUD/RPGHUD.h"
#include "Player/RPGPlayerState.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "AbilitySystem/RPGAttributeSet.h"
#include "UI/WidgetController/RPGWidgetController.h"
#include "Game/RPGGameModeBase.h"
#include "RPGAbilityTypes.h"
#include "Interaction/CombatInterface.h"

/// <summary>
/// 
/// </summary>
/// <param name="WorldContextObject"></param>
/// <returns></returns>
UOverlayWidgetController* URPGAbilitySystemLibrary::GetOverlayWidgetController(const UObject* WorldContextObject)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		if (ARPGHUD* RPGHUD = Cast<ARPGHUD>(PC->GetHUD()))
		{
			ARPGPlayerState* PS = PC->GetPlayerState<ARPGPlayerState>();
			UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
			UAttributeSet* AS = PS->GetAttributeSet();

			const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);

			return RPGHUD->GetOverlayWidgetController(WidgetControllerParams);
		}
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
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		if (ARPGHUD* RPGHUD = Cast<ARPGHUD>(PC->GetHUD()))
		{
			ARPGPlayerState* PS = PC->GetPlayerState<ARPGPlayerState>();
			UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
			UAttributeSet* AS = PS->GetAttributeSet();

			const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);

			return RPGHUD->GetAttributeMenuWidgetController(WidgetControllerParams);
		}
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
		if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(ASC->GetAvatarActor()))
		{
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, CombatInterface->GetPlayerLevel());

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

bool URPGAbilitySystemLibrary::IsNotFriend(AActor* FirstActor, AActor* SecondActor)
{
	// Both Actors have player tag
	const bool bBothArePlayers = FirstActor->ActorHasTag(FName("Player")) && SecondActor->ActorHasTag(FName("Player"));

	// Both Actors have enemy tag
	const bool bBothAreEnemies = FirstActor->ActorHasTag(FName("Enemy")) && SecondActor->ActorHasTag(FName("Enemy"));

	const bool bFriends = bBothArePlayers || bBothAreEnemies;

	return !bFriends;
}
