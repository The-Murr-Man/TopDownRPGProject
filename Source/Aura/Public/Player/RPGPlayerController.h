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
class AMagicCircle;
class IHighlightInterface;


enum class ETargetingStatus : uint8
{
	TargetingEnemy,
	TargetingNonEnemy,
	NotTargeting
};

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

	void SetShowMouseCursorAndForceRefresh(bool bNewValue);

	UFUNCTION(BlueprintCallable)
	void ShowMagicCircle(UMaterialInterface* DecalMaterial = nullptr);

	UFUNCTION(BlueprintCallable)
	void HideMagicCircle();

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
	static void HighlightActor(AActor* InActor);
	static void UnHighlightActor(AActor* InActor);

	void UpdateMagicCircleLocation();

	UPROPERTY(EditAnywhere, Category = "Input")
	bool bUseClickToMove = false;

	//Input
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> RPGContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> ShiftAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<URPGInputConfig> InputConfig;

	// Returns Player Ability System Component
	URPGAbilitySystemComponent* GetASC();

	UPROPERTY()
	TObjectPtr<URPGAbilitySystemComponent> RPGAbilitySystemComponent;

	TObjectPtr<AActor> LastActor;
	TObjectPtr<AActor> ThisActor;

	FHitResult CursorHit;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UDamageTextComponent> DamageTextComponentClass;

	// Click To Move Variables
	bool bShiftKeyDown = false;

	FVector CachedDestination = FVector::ZeroVector;
	float FollowTime = 0;
	float ShortPressThreshold = 0.5f;
	bool bAutoRunning = false;

	ETargetingStatus TargetingStatus = ETargetingStatus::NotTargeting;

	UPROPERTY(EditDefaultsOnly)
	float AutoRunAcceptanceRadius = 50;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USplineComponent> Spline;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UNiagaraSystem> ClickNiagaraSystem;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AMagicCircle> MagicCircleClass;

	UPROPERTY()
	TObjectPtr<AMagicCircle> MagicCircle;
};
