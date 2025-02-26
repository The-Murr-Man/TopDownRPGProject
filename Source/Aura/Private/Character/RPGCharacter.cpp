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
/// 
/// </summary>
void ARPGCharacter::LoadProgress()
{
	ARPGGameModeBase* RPGGameMode = Cast<ARPGGameModeBase>(UGameplayStatics::GetGameMode(this));

	if (!IsValid(RPGGameMode)) return;

	// Gets the save data from the game mode
	ULoadScreenSaveGame* SaveData = RPGGameMode->RetreiveInGameSaveData();

	//if (!SaveData) return;

	if (SaveData->bFirstTimeLoadIn)
	{
		InitializeDefaultAttributes();
		AddCharacterAbilities();
	}

	else
	{
		if (URPGAbilitySystemComponent* RPGASC = Cast<URPGAbilitySystemComponent>(AbilitySystemComponent))
		{
			RPGASC->AddCharacterAbilitiesFromSaveData(SaveData);
		}
		
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
/// 
/// </summary>
void ARPGCharacter::InitAbilityActorInfo()
{
	// Init ability actor info for the server
	ARPGPlayerState* RpgPlayerState = GetPlayerState<ARPGPlayerState>();

	if (!RpgPlayerState) return;
	RpgPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(RpgPlayerState, this);
	Cast<URPGAbilitySystemComponent>(RpgPlayerState->GetAbilitySystemComponent())->AbilityActorInfoSet();

	// Gets and sets ability system component and attribute set from player state
	AbilitySystemComponent = RpgPlayerState->GetAbilitySystemComponent();
	AttributeSet = RpgPlayerState->GetAttributeSet();

	OnASCRegistered.Broadcast(AbilitySystemComponent);
	AbilitySystemComponent->RegisterGameplayTagEvent(FRPGGameplayTags::Get().Debuff_Stun, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ARPGCharacter::StunTagChanged);

	if (ARPGPlayerController* RPGPlayerContoller = Cast<ARPGPlayerController>(GetController()))
	{
		if (ARPGHUD* RPGHUD = Cast<ARPGHUD>(RPGPlayerContoller->GetHUD()))
		{
			RPGHUD->InitOverlay(RPGPlayerContoller, RpgPlayerState, AbilitySystemComponent, AttributeSet);
		}
	}

	//InitializeDefaultAttributes();
}

/// <summary>
/// 
/// </summary>
void ARPGCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// Init ability actor info for the client
	InitAbilityActorInfo();
}

/// <summary>
/// 
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
/// 
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
/// 
/// </summary>
/// <param name="CheckpointTag"></param>
void ARPGCharacter::SaveProgress_Implementation(const FName& CheckpointTag)
{
	ARPGGameModeBase* RPGGameMode = Cast<ARPGGameModeBase>(UGameplayStatics::GetGameMode(this));
	
	if (!IsValid(RPGGameMode)) return;

	// Gets the save data from the game mode
	ULoadScreenSaveGame* SaveData = RPGGameMode->RetreiveInGameSaveData();

	if (!SaveData) return;

	SaveData->PlayerStartTag = CheckpointTag;
	
	if (ARPGPlayerState* RPGPlayerState = Cast<ARPGPlayerState>(GetPlayerState()))
	{
		// Saving data from the player state
		SaveData->PlayerLevel = RPGPlayerState->GetPlayerLevel();
		SaveData->XP = RPGPlayerState->GetPlayerXP();
		SaveData->SpellPoints = RPGPlayerState->GetPlayerSpellPoints();
		SaveData->AttributePoints = RPGPlayerState->GetPlayerAttributePoints();
	}

	//Saving our vital attributes (All other attributes are based on the vital attribute so no need to save them)
	SaveData->StrengthAttribute = URPGAttributeSet::GetStrengthAttribute().GetNumericValue(GetAttributeSet());
	SaveData->IntellegenceAttribute = URPGAttributeSet::GetIntellegenceAttribute().GetNumericValue(GetAttributeSet());
	SaveData->ResilienceAttribute = URPGAttributeSet::GetResilienceAttribute().GetNumericValue(GetAttributeSet());
	SaveData->VigorAttribute = URPGAttributeSet::GetVigorAttribute().GetNumericValue(GetAttributeSet());

	SaveData->bFirstTimeLoadIn = false;

	if (!HasAuthority()) return;

	URPGAbilitySystemComponent* RPGASC = Cast<URPGAbilitySystemComponent>(AbilitySystemComponent);

	FForEachAbility SaveAbilityDelegate;

	// Empty out the array
	SaveData->SavedAbilities.Empty();

	SaveAbilityDelegate.BindLambda([this, RPGASC, SaveData](const FGameplayAbilitySpec& AbilitySpec)
	{
		const FGameplayTag AbilityTag = RPGASC->GetAbilityTagFromSpec(AbilitySpec);

		UAbilityInfo* AbilityInfo = URPGAbilitySystemLibrary::GetAbilityInfo(this);

		FRPGAbilityInfo RPGInfo = AbilityInfo->FindAbilityInfoForTag(AbilityTag);

		FSavedAbility SavedAbility;
		
		SavedAbility.GameplayAbility = RPGInfo.Ability;
		SavedAbility.AbilityLevel = AbilitySpec.Level;
		SavedAbility.AbilitySlot = RPGASC->GetSlotFromAbilityTag(AbilityTag);
		SavedAbility.AbilityStatus = RPGASC->GetStatusFromAbilityTag(AbilityTag);
		SavedAbility.AbilityTag = AbilityTag;
		SavedAbility.AbilityType = RPGInfo.AbilityType;
		SaveData->SavedAbilities.AddUnique(SavedAbility);
	});

	// Broadcasts the SaveAbilityDelegate for each ability in our ability array
	RPGASC->ForEachAbility(SaveAbilityDelegate);


	// Sends our save data to the game mode which saves the progress
	RPGGameMode->SaveInGameProgressData(SaveData);
}

/// <summary>
/// 
/// </summary>
/// <returns></returns>
int32 ARPGCharacter::GetPlayerLevel_Implementation()
{
	ARPGPlayerState* RpgPlayerState = GetPlayerState<ARPGPlayerState>();

	check(RpgPlayerState);
	return RpgPlayerState->GetPlayerLevel();
}

/// <summary>
/// 
/// </summary>
/// <param name="DeathImpulse"></param>
void ARPGCharacter::Die(const FVector& DeathImpulse)
{
	Super::Die(DeathImpulse);

	// Handle Death
	FTimerDelegate DeathTimerDelegate;

	// TODO: Swap To Callback Function
	DeathTimerDelegate.BindLambda([this]()
	{
		ARPGGameModeBase* RPGGameMode = Cast<ARPGGameModeBase>(UGameplayStatics::GetGameMode(this));
		if (RPGGameMode)
		{
			RPGGameMode->PlayerDied(this);
		}
	});

	GetWorldTimerManager().SetTimer(DeathTimer, DeathTimerDelegate, DeathTime, false);
	CameraComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

}

/// <summary>
/// 
/// </summary>
/// <param name="InXP"></param>
void ARPGCharacter::AddToXP_Implementation(int32 InXP)
{
	ARPGPlayerState* RpgPlayerState = GetPlayerState<ARPGPlayerState>();
	check(RpgPlayerState);

	RpgPlayerState->AddToXP(InXP);
}

/// <summary>
/// 
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
/// 
/// </summary>
/// <param name="InAttributePoints"></param>
void ARPGCharacter::AddToAttributePoints_Implementation(int32 InAttributePoints)
{
	ARPGPlayerState* RpgPlayerState = GetPlayerState<ARPGPlayerState>();
	check(RpgPlayerState);

	RpgPlayerState->AddToAttributePoints(InAttributePoints);
}

void ARPGCharacter::AddToSpellPoints_Implementation(int32 InSpellPoints)
{
	ARPGPlayerState* RpgPlayerState = GetPlayerState<ARPGPlayerState>();
	check(RpgPlayerState);

	RpgPlayerState->AddToSpellPoints(InSpellPoints);
}

/// <summary>
/// 
/// </summary>
void ARPGCharacter::LevelUp_Implementation()
{
	MulticastLevelUpParticles();
}

/// <summary>
/// 
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
/// 
/// </summary>
void ARPGCharacter::OnRep_Stunned()
{
	if (URPGAbilitySystemComponent* RPGASC = Cast<URPGAbilitySystemComponent>(AbilitySystemComponent))
	{
		const FRPGGameplayTags GameplayTags = FRPGGameplayTags::Get();
		FGameplayTagContainer BlockedTags;
		BlockedTags.AddTag(GameplayTags.Player_Block_CursorTrace);
		BlockedTags.AddTag(GameplayTags.Player_Block_InputHeld);
		BlockedTags.AddTag(GameplayTags.Player_Block_InputPressed);
		BlockedTags.AddTag(GameplayTags.Player_Block_InputReleased);

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
/// 
/// </summary>
/// <returns></returns>
int32 ARPGCharacter::GetXP_Implementation() const
{
	ARPGPlayerState* RpgPlayerState = GetPlayerState<ARPGPlayerState>();
	check(RpgPlayerState);
	return RpgPlayerState->GetPlayerXP();;
}

/// <summary>
/// 
/// </summary>
/// <returns></returns>
int32 ARPGCharacter::GetAttributePoints_Implementation() const
{
	ARPGPlayerState* RpgPlayerState = GetPlayerState<ARPGPlayerState>();
	check(RpgPlayerState);
	return RpgPlayerState->GetPlayerAttributePoints();
}

/// <summary>
/// 
/// </summary>
/// <returns></returns>
int32 ARPGCharacter::GetSpellPoints_Implementation() const
{
	ARPGPlayerState* RpgPlayerState = GetPlayerState<ARPGPlayerState>();
	check(RpgPlayerState);
	return RpgPlayerState->GetPlayerSpellPoints();
}

/// <summary>
/// 
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
/// 
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
/// 
/// </summary>
/// <param name="InXP"></param>
/// <returns></returns>
int32 ARPGCharacter::FindLevelForXP_Implementation(int32 InXP) const
{
	ARPGPlayerState* RpgPlayerState = GetPlayerState<ARPGPlayerState>();
	check(RpgPlayerState);

	return RpgPlayerState->LevelUpInfo->FindLevelForXP(InXP);
}









