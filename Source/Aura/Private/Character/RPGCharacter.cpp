// Copyright Kyle Murray


#include "Character/RPGCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/RPGPlayerState.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "Player/RPGPlayerController.h"
#include "UI/HUD/RPGHUD.h"


ARPGCharacter::ARPGCharacter()
{
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0, 400, 0);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;
}

void ARPGCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Init ability actor info for the server
	InitAbilityActorInfo();
	AddCharacterAbilities();
}

void ARPGCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// Init ability actor info for the client
	InitAbilityActorInfo();
}

int32 ARPGCharacter::GetPlayerLevel()
{
	ARPGPlayerState* RpgPlayerState = GetPlayerState<ARPGPlayerState>();

	check(RpgPlayerState);
	return RpgPlayerState->GetPlayerLevel();
}

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

	
	if (ARPGPlayerController* RPGPlayerContoller = Cast<ARPGPlayerController>(GetController()))
	{
		if (ARPGHUD* RPGHUD = Cast<ARPGHUD>(RPGPlayerContoller->GetHUD()))
		{
			RPGHUD->InitOverlay(RPGPlayerContoller, RpgPlayerState, AbilitySystemComponent, AttributeSet);
		}
	}

	InitializeDefaultAttributes();
}

