// Copyright Kyle Murray


#include "Player/RPGPlayerController.h"
#include "EnhancedInputSubsystems.h"

ARPGPlayerController::ARPGPlayerController()
{
	bReplicates = true;
}

void ARPGPlayerController::BeginPlay()
{
	Super::BeginPlay();

	check(RPGContext);

	// This is a singleton used to add mapping contexts for our controller
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	check(Subsystem);

	Subsystem->AddMappingContext(RPGContext, 0);

	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;

	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetHideCursorDuringCapture(false);

	SetInputMode(InputModeData);
}