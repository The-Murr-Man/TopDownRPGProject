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

void ARPGPlayerController::ShowDamageNumber_Implementation(float DamageAmount, ACharacter* TargetCharacter)
{
	if (IsValid(TargetCharacter) && DamageTextComponentClass)
	{
		// Creating new Damage Text Comp
		UDamageTextComponent* DamageText = NewObject<UDamageTextComponent>(TargetCharacter, DamageTextComponentClass);
		DamageText->RegisterComponent();

		// Attaches DamageText to targets root component
		DamageText->AttachToComponent(TargetCharacter->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

		// Detaches DamageText
		DamageText->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

		// Sets DamageText to DamageAmount
		DamageText->SetDamageText(DamageAmount);
	}
}

void ARPGPlayerController::CursorTrace()
{
	GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);

	if (!CursorHit.bBlockingHit) return;

	LastActor = ThisActor;
	ThisActor = CursorHit.GetActor();

	if (LastActor != ThisActor)
	{
		if (LastActor) LastActor->UnHighlightActor();
		
		if (ThisActor) ThisActor->HighlightActor();
	}
}

void ARPGPlayerController::AutoRun()
{
	if (bUseClickToMove)
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
}

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


void ARPGPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
	// Only do this if you want to use Click to move
	if (bUseClickToMove)
	{
		if (InputTag.MatchesTagExact(FRPGGameplayTags::Get().InputTag_LMB))
		{
			bTargeting = ThisActor ? true : false;
			bAutoRunning = false;
		}
	}
}

void ARPGPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (!GetASC()) return;

	// If not Using Click to move
	if (!bUseClickToMove) GetASC()->AbilityInputTagReleased(InputTag);
	
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
		if (!bTargeting && !bShiftKeyDown)
		{
			APawn* ControlledPawn = GetPawn();

			if (FollowTime <= ShortPressThreshold && ControlledPawn)
			{
				if (UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(this, ControlledPawn->GetActorLocation(), CachedDestination))
				{
					Spline->ClearSplinePoints();

					for (const FVector& PointLoc : NavPath->PathPoints)
					{
						Spline->AddSplinePoint(PointLoc, ESplineCoordinateSpace::World);
					}

					CachedDestination = NavPath->PathPoints[NavPath->PathPoints.Num() - 1];
					bAutoRunning = true;
				}
			}

			FollowTime = 0;
			bTargeting = false;
		}
	}
}

void ARPGPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
	if (!GetASC()) return;
	
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
		if (bTargeting||bShiftKeyDown) GetASC()->AbilityInputTagHeld(InputTag);
		
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

URPGAbilitySystemComponent* ARPGPlayerController::GetASC()
{
	if (!RPGAbilitySystemComponent)
	{
		// Setting our ability system component to the result of GetAbilitySystemComponent() from the AbilitySystemBlueprintLibrary
		RPGAbilitySystemComponent = Cast<URPGAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>()));
	}

	return RPGAbilitySystemComponent;
}


