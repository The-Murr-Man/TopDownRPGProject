// Copyright Kyle Murray


#include "Character/RPGCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/RPGPlayerState.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "Player/RPGPlayerController.h"
#include "UI/HUD/RPGHUD.h"
#include "AbilitySystem/Data/LevelUpInfo.h"
#include "NiagaraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

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
	AddCharacterAbilities();
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

	if (ARPGPlayerController* RPGPlayerContoller = Cast<ARPGPlayerController>(GetController()))
	{
		if (ARPGHUD* RPGHUD = Cast<ARPGHUD>(RPGPlayerContoller->GetHUD()))
		{
			RPGHUD->InitOverlay(RPGPlayerContoller, RpgPlayerState, AbilitySystemComponent, AttributeSet);
		}
	}

	InitializeDefaultAttributes();
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







