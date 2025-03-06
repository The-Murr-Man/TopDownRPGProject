// Copyright Kyle Murray


#include "Character/RPGCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/RPGPlayerState.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "AbilitySystem/RPGAbilitySystemLibrary.h"
#include "Player/RPGPlayerController.h"
#include "UI/HUD/RPGHUD.h"
#include "AbilitySystem/Data/LevelUpInfo.h"
#include "NiagaraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "RPGGameplayTags.h"
#include "AbilitySystem/Debuff/DebuffNiagaraComponent.h"
#include "Game/RPGGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Game/RPGGameInstance.h"
#include "Game/LoadScreenSaveGame.h"
#include "AbilitySystem/RPGAttributeSet.h"
#include "GameplayEffectExecutionCalculation.h"

/// <summary>
/// 
/// </summary>
ARPGCharacter::ARPGCharacter()
{
	LevelUpNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>("LevelUpNiagaraComponent");
	LevelUpNiagaraComponent->SetupAttachment(GetRootComponent());
	LevelUpNiagaraComponent->bAutoActivate = false;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>("CameraBoom");
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->SetUsingAbsoluteRotation(true);
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->TargetArmLength = 800;
	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>("CameraComponent");
	CameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	CameraComponent->bUsePawnControlRotation = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0, 400, 0);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	CharacterClass = ECharacterClass::Elementalist;
}

/// <summary>
/// 
/// </summary>
/// <param name="NewController"></param>
void ARPGCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Init ability actor info for the server
	InitAbilityActorInfo();

	// Load Save Data
	LoadProgress();

	if (ARPGGameModeBase* RPGGameMode = Cast<ARPGGameModeBase>(UGameplayStatics::GetGameMode(this)))
	{
		RPGGameMode->LoadWorldState(GetWorld());
	}
}

/// <summary>
/// Loads progress from Save Data on GameMode
/// </summary>
void ARPGCharacter::LoadProgress()
{
	ARPGGameModeBase* RPGGameMode = Cast<ARPGGameModeBase>(UGameplayStatics::GetGameMode(this));

	if (!IsValid(RPGGameMode)) return;

	// Gets the save data from the game mode
	ULoadScreenSaveGame* SaveData = RPGGameMode->RetreiveInGameSaveData();

	//if (!SaveData) return;

	// Check if the its the player first time loading in
	if (SaveData->bFirstTimeLoadIn)
	{
		// Add default Attributes and abilities
		InitializeDefaultAttributes();
		AddCharacterAbilities();
	}

	// Load from save data
	else
	{
		// Cast to custom ASC
		if (URPGAbilitySystemComponent* RPGASC = Cast<URPGAbilitySystemComponent>(AbilitySystemComponent))
		{
			// Add abilities from save data
			RPGASC->AddCharacterAbilitiesFromSaveData(SaveData);
		}
		
		// Cast to cutsom PC
		if (ARPGPlayerState* RPGPlayerState = Cast<ARPGPlayerState>(GetPlayerState()))
		{
			// Saving data from the player state
			RPGPlayerState->SetPlayerLevel(SaveData->PlayerLevel);
			RPGPlayerState->SetPlayerXP(SaveData->XP);
			RPGPlayerState->SetPlayerSpellPoints(SaveData->SpellPoints);
			RPGPlayerState->SetPlayerAttributePoints(SaveData->AttributePoints);
		}

		URPGAbilitySystemLibrary::InitializeDefaultAttributesFromSaveData(this, AbilitySystemComponent, SaveData);
	}
}

/// <summary>
/// Initialize Actors Ability Info
/// </summary>
void ARPGCharacter::InitAbilityActorInfo()
{
	// Init ability actor info for the server
	ARPGPlayerState* RpgPlayerState = GetPlayerState<ARPGPlayerState>();

	if (!RpgPlayerState) return;

	// Call ASC Version
	RpgPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(RpgPlayerState, this);

	Cast<URPGAbilitySystemComponent>(RpgPlayerState->GetAbilitySystemComponent())->AbilityActorInfoSet();

	// Gets and sets ability system component and attribute set from player state
	AbilitySystemComponent = RpgPlayerState->GetAbilitySystemComponent();
	AttributeSet = RpgPlayerState->GetAttributeSet();

	// Broadcast ASC Registered
	OnASCRegistered.Broadcast(AbilitySystemComponent);
	AbilitySystemComponent->RegisterGameplayTagEvent(FRPGGameplayTags::Get().Debuff_Stun, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ARPGCharacter::StunTagChanged);

	// Cast to Custom PC
	if (ARPGPlayerController* RPGPlayerContoller = Cast<ARPGPlayerController>(GetController()))
	{
		// Cast to HUD
		if (ARPGHUD* RPGHUD = Cast<ARPGHUD>(RPGPlayerContoller->GetHUD()))
		{
			// Initialize our overlay
			RPGHUD->InitOverlay(RPGPlayerContoller, RpgPlayerState, AbilitySystemComponent, AttributeSet);
		}
	}
}

/// <summary>
/// Replicate PlayerState
/// </summary>
void ARPGCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// Init ability actor info for the client
	InitAbilityActorInfo();
}

/// <summary>
/// Shows Magic Cicle
/// </summary>
/// <param name="DecalMaterial"></param>
void ARPGCharacter::ShowMagicCircle_Implementation(UMaterialInterface* DecalMaterial)
{
	if (!IsLocallyControlled()) return;

	if (ARPGPlayerController* RPGPlayerContoller = Cast<ARPGPlayerController>(GetController()))
	{
		RPGPlayerContoller->ShowMagicCircle(DecalMaterial);
		RPGPlayerContoller->SetShowMouseCursorAndForceRefresh(false);
	}
}

/// <summary>
/// Hide Magic Circle
/// </summary>
void ARPGCharacter::HideMagicCircle_Implementation()
{
	if (!IsLocallyControlled()) return;

	if (ARPGPlayerController* RPGPlayerContoller = Cast<ARPGPlayerController>(GetController()))
	{
		RPGPlayerContoller->HideMagicCircle();
		RPGPlayerContoller->SetShowMouseCursorAndForceRefresh(true);
	}
}

/// <summary>
/// Save Player Progress
/// </summary>
/// <param name="CheckpointTag"></param>
void ARPGCharacter::SaveProgress_Implementation(const FName& CheckpointTag)
{
	ARPGGameModeBase* RPGGameMode = Cast<ARPGGameModeBase>(UGameplayStatics::GetGameMode(this));
	
	if (!IsValid(RPGGameMode)) return;

	// Gets the save data from the game mode
	ULoadScreenSaveGame* SaveData = RPGGameMode->RetreiveInGameSaveData();

	if (!SaveData) return;

	// Sets checkpoint tag
	SaveData->PlayerStartTag = CheckpointTag;
	
	if (ARPGPlayerState* RPGPlayerState = Cast<ARPGPlayerState>(GetPlayerState()))
	{
		// Saving data from the player state
		SaveData->PlayerLevel = RPGPlayerState->GetPlayerLevel();
		SaveData->XP = RPGPlayerState->GetPlayerXP();
		SaveData->SpellPoints = RPGPlayerState->GetPlayerSpellPoints();
		SaveData->AttributePoints = RPGPlayerState->GetPlayerAttributePoints();
	}

	// Remove Any Infinite Effects that boost Attributes
	URPGAbilitySystemComponent* RPGASC = Cast<URPGAbilitySystemComponent>(AbilitySystemComponent);

	//Saving our vital attributes (All other attributes are based on the vital attribute so no need to save them)
	SaveData->StrengthAttribute = URPGAttributeSet::GetStrengthAttribute().GetGameplayAttributeData(GetAttributeSet())->GetBaseValue();
	SaveData->IntellegenceAttribute = URPGAttributeSet::GetIntellegenceAttribute().GetGameplayAttributeData(GetAttributeSet())->GetBaseValue();
	SaveData->ResilienceAttribute = URPGAttributeSet::GetResilienceAttribute().GetGameplayAttributeData(GetAttributeSet())->GetBaseValue();
	SaveData->VigorAttribute = URPGAttributeSet::GetVigorAttribute().GetGameplayAttributeData(GetAttributeSet())->GetBaseValue();

	SaveData->bFirstTimeLoadIn = false;

	if (!HasAuthority()) return;

	FForEachAbility SaveAbilityDelegate;

	// Empty out the array
	SaveData->SavedAbilities.Empty();

	// Saves Ability
	SaveAbilityDelegate.BindLambda([this, RPGASC, SaveData](const FGameplayAbilitySpec& AbilitySpec)
	{
		SaveAbility(RPGASC, AbilitySpec, SaveData);
	});

	// Broadcasts the SaveAbilityDelegate for each ability in our ability array
	RPGASC->ForEachAbility(SaveAbilityDelegate);


	// Sends our save data to the game mode which saves the progress
	RPGGameMode->SaveInGameProgressData(SaveData);
}

/// <summary>
/// Saves character abilities
/// </summary>
/// <param name="RPGASC"></param>
/// <param name="AbilitySpec"></param>
/// <param name="SaveData"></param>
void ARPGCharacter::SaveAbility(URPGAbilitySystemComponent* RPGASC, const FGameplayAbilitySpec& AbilitySpec, ULoadScreenSaveGame* SaveData)
{
	// Create needed variables
	const FGameplayTag AbilityTag = RPGASC->GetAbilityTagFromSpec(AbilitySpec);
	UAbilityInfo* AbilityInfo = URPGAbilitySystemLibrary::GetAbilityInfo(this);
	FRPGAbilityInfo RPGInfo = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
	FSavedAbility SavedAbility;

	// Set all SavedAbility infomation
	SavedAbility.GameplayAbility = RPGInfo.Ability;
	SavedAbility.AbilityLevel = AbilitySpec.Level;
	SavedAbility.AbilitySlot = RPGASC->GetSlotFromAbilityTag(AbilityTag);
	SavedAbility.AbilityStatus = RPGASC->GetStatusFromAbilityTag(AbilityTag);
	SavedAbility.AbilityTag = AbilityTag;
	SavedAbility.AbilityType = RPGInfo.AbilityType;

	// Add SavedAbility to SavedAbilities
	SaveData->SavedAbilities.AddUnique(SavedAbility);
}

/// <summary>
/// Returns Player Level
/// </summary>
/// <returns></returns>
int32 ARPGCharacter::GetPlayerLevel_Implementation()
{
	ARPGPlayerState* RpgPlayerState = GetPlayerState<ARPGPlayerState>();

	check(RpgPlayerState);
	return RpgPlayerState->GetPlayerLevel();
}

/// <summary>
/// Handles character death
/// </summary>
/// <param name="DeathImpulse"></param>
void ARPGCharacter::Die(const FVector& DeathImpulse)
{
	Super::Die(DeathImpulse);

	// Handle Death
	FTimerDelegate DeathTimerDelegate;

	// Calls GameModes PlayerDied
	DeathTimerDelegate.BindLambda([this]()
	{
		ARPGGameModeBase* RPGGameMode = Cast<ARPGGameModeBase>(UGameplayStatics::GetGameMode(this));
		if (RPGGameMode)
		{
			RPGGameMode->PlayerDied(this);
		}
	});

	// Sets timer
	GetWorldTimerManager().SetTimer(DeathTimer, DeathTimerDelegate, DeathTime, false);

	// Detaches Camera
	CameraComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
}

/// <summary>
/// Adds To Experience
/// </summary>
/// <param name="InXP"></param>
void ARPGCharacter::AddToXP_Implementation(int32 InXP)
{
	ARPGPlayerState* RpgPlayerState = GetPlayerState<ARPGPlayerState>();
	check(RpgPlayerState);

	RpgPlayerState->AddToXP(InXP);
}

/// <summary>
/// Adds to the players level
/// </summary>
/// <param name="InPlayerLevel"></param>
void ARPGCharacter::AddToPlayerLevel_Implementation(int32 InPlayerLevel)
{
	ARPGPlayerState* RpgPlayerState = GetPlayerState<ARPGPlayerState>();
	check(RpgPlayerState);

	RpgPlayerState->AddToLevel(InPlayerLevel);

	if (URPGAbilitySystemComponent* RPGASC = Cast<URPGAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		RPGASC->UpdateAbilityStatuses(RpgPlayerState->GetPlayerLevel());
	}

}

/// <summary>
/// Adds to attributes points
/// </summary>
/// <param name="InAttributePoints"></param>
void ARPGCharacter::AddToAttributePoints_Implementation(int32 InAttributePoints)
{
	ARPGPlayerState* RpgPlayerState = GetPlayerState<ARPGPlayerState>();
	check(RpgPlayerState);

	RpgPlayerState->AddToAttributePoints(InAttributePoints);
}

/// <summary>
/// Adds to spell points
/// </summary>
/// <param name="InSpellPoints"></param>
void ARPGCharacter::AddToSpellPoints_Implementation(int32 InSpellPoints)
{
	ARPGPlayerState* RpgPlayerState = GetPlayerState<ARPGPlayerState>();
	check(RpgPlayerState);

	RpgPlayerState->AddToSpellPoints(InSpellPoints);
}

/// <summary>
/// Handles Level UP
/// </summary>
void ARPGCharacter::LevelUp_Implementation()
{
	MulticastLevelUpParticles();
}

/// <summary>
/// Handles the particles for level up
/// </summary>
void ARPGCharacter::MulticastLevelUpParticles_Implementation() const
{
	if (!IsValid(LevelUpNiagaraComponent)) return;

	const FVector CameraLocation = CameraComponent->GetComponentLocation();
	const FVector NiagaraSystemLocation = LevelUpNiagaraComponent->GetComponentLocation();

	const FRotator ToCameraRotation = (CameraLocation - NiagaraSystemLocation).Rotation();

	LevelUpNiagaraComponent->SetWorldRotation(ToCameraRotation);
	LevelUpNiagaraComponent->Activate(true);
}

/// <summary>
/// Replicated function for setting Stunned
/// </summary>
void ARPGCharacter::OnRep_Stunned()
{
	if (URPGAbilitySystemComponent* RPGASC = Cast<URPGAbilitySystemComponent>(AbilitySystemComponent))
	{
		const FRPGGameplayTags GameplayTags = FRPGGameplayTags::Get();
		FGameplayTagContainer BlockedTags;

		// Blocks input
		BlockedTags.AddTag(GameplayTags.Player_Block_CursorTrace);
		BlockedTags.AddTag(GameplayTags.Player_Block_InputHeld);
		BlockedTags.AddTag(GameplayTags.Player_Block_InputPressed);
		BlockedTags.AddTag(GameplayTags.Player_Block_InputReleased);

		// Check if Stunned
		if (bIsStunned)
		{
			RPGASC->AddLooseGameplayTags(BlockedTags);
			StunDebuffComponent->Activate();
		}

		else
		{
			RPGASC->RemoveLooseGameplayTags(BlockedTags);
			StunDebuffComponent->Deactivate();
		}
	}
}

/// <summary>
/// Replicated function for setting Burned
/// </summary>
void ARPGCharacter::OnRep_Burned()
{
	if (bIsBurned)
	{
		BurnDebuffComponent->Activate();
	}

	else
	{
		BurnDebuffComponent->Deactivate();
	}
}

/// <summary>
/// Returns XP
/// </summary>
/// <returns></returns>
int32 ARPGCharacter::GetXP_Implementation() const
{
	ARPGPlayerState* RpgPlayerState = GetPlayerState<ARPGPlayerState>();
	check(RpgPlayerState);
	return RpgPlayerState->GetPlayerXP();;
}

/// <summary>
/// Returns spell points
/// </summary>
/// <returns></returns>
int32 ARPGCharacter::GetAttributePoints_Implementation() const
{
	ARPGPlayerState* RpgPlayerState = GetPlayerState<ARPGPlayerState>();
	check(RpgPlayerState);
	return RpgPlayerState->GetPlayerAttributePoints();
}

/// <summary>
///  Returns spell points
/// </summary>
/// <returns></returns>
int32 ARPGCharacter::GetSpellPoints_Implementation() const
{
	ARPGPlayerState* RpgPlayerState = GetPlayerState<ARPGPlayerState>();
	check(RpgPlayerState);
	return RpgPlayerState->GetPlayerSpellPoints();
}

/// <summary>
///  Returns attribute points rewarded for given level
/// </summary>
/// <param name="Level"></param>
/// <returns></returns>
int32 ARPGCharacter::GetAttributePointsReward_Implementation(int32 Level) const
{
	ARPGPlayerState* RpgPlayerState = GetPlayerState<ARPGPlayerState>();
	check(RpgPlayerState);

	return RpgPlayerState->LevelUpInfo->LevelUpInformation[Level].AttributePointReward;
}

/// <summary>
/// Returns Spell points rewarded for given level
/// </summary>
/// <param name="Level"></param>
/// <returns></returns>
int32 ARPGCharacter::GetSpellPointsReward_Implementation(int32 Level) const
{
	ARPGPlayerState* RpgPlayerState = GetPlayerState<ARPGPlayerState>();
	check(RpgPlayerState);

	return RpgPlayerState->LevelUpInfo->LevelUpInformation[Level].SpellPointReward;
}

/// <summary>
/// Return Level for given XP
/// </summary>
/// <param name="InXP"></param>
/// <returns></returns>
int32 ARPGCharacter::FindLevelForXP_Implementation(int32 InXP) const
{
	ARPGPlayerState* RpgPlayerState = GetPlayerState<ARPGPlayerState>();
	check(RpgPlayerState);

	return RpgPlayerState->LevelUpInfo->FindLevelForXP(InXP);
}
