// Copyright Kyle Murray

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "RPGPlayerController.generated.h"

// Forward Declarations
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class IEnemyInterface;
class URPGInputConfig;
class URPGAbilitySystemComponent;
class USplineComponent;
class UDamageTextComponent;
class UNiagaraSystem;

/**
 * 
 */
UCLASS()
class AURA_API ARPGPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ARPGPlayerController();

	virtual void PlayerTick(float DeltaTime) override;

	UFUNCTION(Client,Reliable)
	void ShowDamageNumber(float DamageAmount, ACharacter* TargetCharacter, bool bBlockedHit, bool bCriticalHit);

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:

	// Function Callbacks
	void Move(const FInputActionValue& InputActionValue);

	//
	void ShiftPressed() { bShiftKeyDown = true; };
	void ShiftReleased() { bShiftKeyDown = false; };

	void AutoRun();

	// Ability Callbacks
	void AbilityInputTagPressed(FGameplayTag InputTag);
	void AbilityInputTagReleased(FGameplayTag InputTag);
	void AbilityInputTagHeld(FGameplayTag InputTag);

	void CursorTrace();

	UPROPERTY(EditAnywhere, Category = "Input")
	bool bUseClickToMove = false;

	// Returns Player Ability System Component
	URPGAbilitySystemComponent* GetASC();

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> RPGContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> ShiftAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<URPGInputConfig> InputConfig;

	UPROPERTY()
	TObjectPtr<URPGAbilitySystemComponent> RPGAbilitySystemComponent;

	TScriptInterface<IEnemyInterface> LastActor;
	TScriptInterface<IEnemyInterface> ThisActor;

	FHitResult CursorHit;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UDamageTextComponent> DamageTextComponentClass;

	// Click To Move Variables
	bool bShiftKeyDown = false;

	FVector CachedDestination = FVector::ZeroVector;
	float FollowTime = 0;
	float ShortPressThreshold = 0.5f;
	bool bAutoRunning = false;
	bool bTargeting = false;

	UPROPERTY(EditDefaultsOnly)
	float AutoRunAcceptanceRadius = 50;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USplineComponent> Spline;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UNiagaraSystem> ClickNiagaraSystem;
};
