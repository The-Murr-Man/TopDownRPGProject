// Copyright Kyle Murray


#include "Player/RPGPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Interaction/EnemyInterface.h"
#include "Input/RPGInputComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "Components/SplineComponent.h"
#include "RPGGameplayTags.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "GameFramework/Character.h"
#include "UI/Widget/DamageTextComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Actor/MagicCircle.h"
#include "Aura/Aura.h"
#include "Interaction/HighlightInterface.h"

ARPGPlayerController::ARPGPlayerController()
{
	bReplicates = true;

	Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
}

void ARPGPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	CursorTrace();

	if (bUseClickToMove)
	{
		AutoRun();
	}

	UpdateMagicCircleLocation();
}

void ARPGPlayerController::BeginPlay()
{
	Super::BeginPlay();

	check(RPGContext);

	// This is a singleton used to add mapping contexts for our controller
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (Subsystem)
	{
		Subsystem->AddMappingContext(RPGContext, 0);
	}

	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;

	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetHideCursorDuringCapture(false);

	SetInputMode(InputModeData);
}

void ARPGPlayerController::SetShowMouseCursorAndForceRefresh(bool bNewValue)
{
	SetShowMouseCursor(bNewValue);
	// Workaround: Force the cursor refresh by setting the mouse position to itself.
	float XMouseLocation;
	float YMouseLocation;
	GetMousePosition(XMouseLocation, YMouseLocation);
	SetMouseLocation(XMouseLocation, YMouseLocation);
}

/// <summary>
/// 
/// </summary>
/// <param name="DecalMaterial"></param>
void ARPGPlayerController::ShowMagicCircle(UMaterialInterface* DecalMaterial)
{
	if (!IsValid(MagicCircle))
	{
		FVector MagicCircleLoc = CursorHit.ImpactPoint;
		MagicCircle = GetWorld()->SpawnActor<AMagicCircle>(MagicCircleClass, MagicCircleLoc, FRotator::ZeroRotator);

		if (DecalMaterial)
		{
			MagicCircle->GetMagicCircleDecal()->SetMaterial(0, DecalMaterial);
		}
	}
}

/// <summary>
/// 
/// </summary>
void ARPGPlayerController::HideMagicCircle()
{
	if (IsValid(MagicCircle))
	{
		MagicCircle->Destroy();
	}
}

/// <summary>
/// 
/// </summary>
/// <param name="DamageAmount"></param>
/// <param name="TargetCharacter"></param>
/// <param name="bBlockedHit"></param>
/// <param name="bCriticalHit"></param>
void ARPGPlayerController::ShowDamageNumber_Implementation(float DamageAmount, ACharacter* TargetCharacter, bool bBlockedHit, bool bCriticalHit)
{
	if (IsValid(TargetCharacter) && DamageTextComponentClass && IsLocalController())
	{
		// Creating new Damage Text Comp
		UDamageTextComponent* DamageText = NewObject<UDamageTextComponent>(TargetCharacter, DamageTextComponentClass);
		DamageText->RegisterComponent();

		// Attaches DamageText to targets root component
		DamageText->AttachToComponent(TargetCharacter->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

		// Detaches DamageText
		DamageText->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

		// Sets DamageText to DamageAmount
		DamageText->SetDamageText(DamageAmount, bBlockedHit,bCriticalHit);
	}
}

/// <summary>
/// 
/// </summary>
void ARPGPlayerController::CursorTrace()
{
	// If blocking cursor trace clear highlight target and return
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FRPGGameplayTags::Get().Player_Block_CursorTrace))
	{
		UnHighlightActor(LastActor);
		UnHighlightActor(ThisActor);

		LastActor = nullptr;
		ThisActor = nullptr;
		return;
	}

	const ECollisionChannel TraceChannel = IsValid(MagicCircle) ? ECC_ExcludePlayers : ECC_Visibility;
	GetHitResultUnderCursor(TraceChannel, false, CursorHit);

	if (!CursorHit.bBlockingHit) return;

	LastActor = ThisActor;

	if (IsValid(CursorHit.GetActor()) && CursorHit.GetActor()->Implements<UHighlightInterface>())
	{
		ThisActor = CursorHit.GetActor();
	}

	else
	{
		ThisActor = nullptr;
	}

	if (LastActor != ThisActor)
	{
		UnHighlightActor(LastActor);
		HighlightActor(ThisActor);
	}
}

/// <summary>
/// 
/// </summary>
/// <param name="InActor"></param>
void ARPGPlayerController::HighlightActor(AActor* InActor)
{
	if (IsValid(InActor) && InActor->Implements<UHighlightInterface>())
	{
		IHighlightInterface::Execute_HighlightActor(InActor);
	}
}

/// <summary>
/// 
/// </summary>
/// <param name="InActor"></param>
void ARPGPlayerController::UnHighlightActor(AActor* InActor)
{
	if (IsValid(InActor) && InActor->Implements<UHighlightInterface>())
	{
		IHighlightInterface::Execute_UnHighlightActor(InActor);
	}
}

/// <summary>
/// 
/// </summary>
void ARPGPlayerController::UpdateMagicCircleLocation()
{
	if (IsValid(MagicCircle))
	{

		if (CursorHit.bBlockingHit)
		{
			MagicCircle->SetActorHiddenInGame(false);
			MagicCircle->SetActorLocation(CursorHit.ImpactPoint);
		}
		else
		{
			MagicCircle->SetActorHiddenInGame(true);
		}
	}
}

/// <summary>
/// 
/// </summary>
void ARPGPlayerController::AutoRun()
{	
	if (!bAutoRunning) return;

	if (APawn* ControlledPawn = GetPawn())
	{
		const FVector LocationOnSpline = Spline->FindLocationClosestToWorldLocation(ControlledPawn->GetActorLocation(), ESplineCoordinateSpace::World);
		const FVector Direction = Spline->FindDirectionClosestToWorldLocation(LocationOnSpline, ESplineCoordinateSpace::World);
		ControlledPawn->AddMovementInput(Direction);

		const float DistanceToDestination = (LocationOnSpline - CachedDestination).Length();

		if (DistanceToDestination <= AutoRunAcceptanceRadius)
		{
			bAutoRunning = false;
		}
	}
}

/// <summary>
/// 
/// </summary>
void ARPGPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	URPGInputComponent* RPGInputComponent = CastChecked<URPGInputComponent>(InputComponent);

	// Binding our move callback function to move action
	if (!bUseClickToMove)
	{
		RPGInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ARPGPlayerController::Move);
		
	}

	else
	{
		RPGInputComponent->BindAction(ShiftAction, ETriggerEvent::Started, this, &ARPGPlayerController::ShiftPressed);
		RPGInputComponent->BindAction(ShiftAction, ETriggerEvent::Completed, this, &ARPGPlayerController::ShiftReleased);
	}

	
	RPGInputComponent->BindAbilityActions(InputConfig, this, &ThisClass::AbilityInputTagPressed, &ThisClass::AbilityInputTagReleased, &ThisClass::AbilityInputTagHeld);
}

/// <summary>
/// 
/// </summary>
/// <param name="InputActionValue"></param>
void ARPGPlayerController::Move(const FInputActionValue& InputActionValue)
{
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		ControlledPawn->AddMovementInput(ForwardDir, InputAxisVector.Y);
		ControlledPawn->AddMovementInput(RightDir, InputAxisVector.X);
	}
}

/// <summary>
/// 
/// </summary>
/// <param name="InputTag"></param>
void ARPGPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FRPGGameplayTags::Get().Player_Block_InputPressed)) return;

	// HARDCODED FOR LEFT MOUSE BUTTON
	if (InputTag.MatchesTagExact(FRPGGameplayTags::Get().InputTag_LMB))
	{
		if (IsValid(ThisActor))
		{
			TargetingStatus = ThisActor->Implements<UEnemyInterface>() ? ETargetingStatus::TargetingEnemy : ETargetingStatus::TargetingNonEnemy;
		}

		else
		{
			TargetingStatus = ETargetingStatus::NotTargeting;
		}

		bAutoRunning = false;
	}

	if (GetASC()) GetASC()->AbilityInputTagPressed(InputTag);
}

/// <summary>
/// 
/// </summary>
/// <param name="InputTag"></param>
void ARPGPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (!GetASC()) return;

	if (GetASC() && GetASC()->HasMatchingGameplayTag(FRPGGameplayTags::Get().Player_Block_InputReleased)) return;

	// If not Using Click to move, use ability TODO: Also check if targeting checkpoint
	if (!bUseClickToMove)
	{
		if (!IsValid(ThisActor)) return;
		
		if ((TargetingStatus == ETargetingStatus::TargetingEnemy || TargetingStatus == ETargetingStatus::NotTargeting) && ThisActor->Implements<UHighlightInterface>())
		{
			GetASC()->AbilityInputTagReleased(InputTag);
		}

		else if(TargetingStatus == ETargetingStatus::TargetingNonEnemy && ThisActor->Implements<UHighlightInterface>())
		{
			IHighlightInterface::Execute_InteractWithActor(ThisActor, this);
		}
	}

	// If Using Click to move
	if (bUseClickToMove)
	{
		if (!InputTag.MatchesTagExact(FRPGGameplayTags::Get().InputTag_LMB))
		{
			GetASC()->AbilityInputTagReleased(InputTag);

			return;
		}
		
		GetASC()->AbilityInputTagReleased(InputTag);

		// If HitResult is not an enemy and not holding shift key
		if (TargetingStatus != ETargetingStatus::TargetingEnemy && !bShiftKeyDown)
		{
			APawn* ControlledPawn = GetPawn();

			if (FollowTime <= ShortPressThreshold && ControlledPawn)
			{
				// If clicking on an actor who can be highlighted
				if (IsValid(ThisActor) && ThisActor->Implements<UHighlightInterface>())
				{
					IHighlightInterface::Execute_SetMoveToLocation(ThisActor, CachedDestination);
				}

				else if (GetASC() && !GetASC()->HasMatchingGameplayTag(FRPGGameplayTags::Get().Player_Block_InputPressed))
				{
					UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ClickNiagaraSystem, CachedDestination);
				}

				if (UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(this, ControlledPawn->GetActorLocation(), CachedDestination))
				{
					Spline->ClearSplinePoints();

					for (const FVector& PointLoc : NavPath->PathPoints)
					{
						Spline->AddSplinePoint(PointLoc, ESplineCoordinateSpace::World);
					}

					if (NavPath->PathPoints.Num() > 0)
					{
						CachedDestination = NavPath->PathPoints[NavPath->PathPoints.Num() - 1];
						bAutoRunning = true;
					}
				}
			}

			FollowTime = 0;
			TargetingStatus = ETargetingStatus::NotTargeting;
		}
	}
}

/// <summary>
/// 
/// </summary>
/// <param name="InputTag"></param>
void ARPGPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
	if (!GetASC()) return;
	
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FRPGGameplayTags::Get().Player_Block_InputHeld)) return;

	// Not using click to move
	if (!bUseClickToMove)
	{
		GetASC()->AbilityInputTagHeld(InputTag);
	}

	// Only do this if you want to use Click to move
	if (bUseClickToMove)
	{
		if (!InputTag.MatchesTagExact(FRPGGameplayTags::Get().InputTag_LMB))
		{
			GetASC()->AbilityInputTagHeld(InputTag);

			return;
		}

		// If HitResult is an enemy or shift key held
		if (TargetingStatus == ETargetingStatus::TargetingEnemy ||bShiftKeyDown) GetASC()->AbilityInputTagHeld(InputTag);
		
		// If HitResult is not an enemy
		else
		{
			FollowTime += GetWorld()->GetDeltaSeconds();

			if(CursorHit.bBlockingHit) CachedDestination = CursorHit.ImpactPoint;
			

			if (APawn* ControlledPawn = GetPawn())
			{
				const FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();

				ControlledPawn->AddMovementInput(WorldDirection);
			}
		}
	}
}

/// <summary>
/// 
/// </summary>
/// <returns></returns>
URPGAbilitySystemComponent* ARPGPlayerController::GetASC()
{
	if (!RPGAbilitySystemComponent)
	{
		// Setting our ability system component to the result of GetAbilitySystemComponent() from the AbilitySystemBlueprintLibrary
		RPGAbilitySystemComponent = Cast<URPGAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>()));
	}

	return RPGAbilitySystemComponent;
}


