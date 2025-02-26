// Copyright Kyle Murray

#pragma once

#include "CoreMinimal.h"
#include "Character/RPGCharacterBase.h"
#include "Interaction/EnemyInterface.h"
#include "Interaction/HighlightInterface.h"
#include "UI/WidgetController/OverlayWidgetController.h"
#include "RPGEnemy.generated.h"

// Forward Declarations
class UWidgetComponent;
class UBehaviorTree;
class ARPGAIController;
/**
 * 
 */
UCLASS()
class AURA_API ARPGEnemy : public ARPGCharacterBase, public IEnemyInterface, public IHighlightInterface
{
	GENERATED_BODY()
	
public:
	ARPGEnemy();

	virtual void PossessedBy(AController* NewController) override;

	// Highlight Interface functions
	virtual void HighlightActor_Implementation() override;
	virtual void UnHighlightActor_Implementation() override;
	virtual void SetMoveToLocation_Implementation(FVector& OutLocation) override;
	//

	// Combat Interface
	virtual void SetCombatTarget_Implementation(AActor* InCombatTarget) override;
	virtual AActor* GetCombatTarget_Implementation() const override;
	virtual int32 GetPlayerLevel_Implementation() override;
	virtual void Die(const FVector& DeathImpulse) override;
	//

	// Callback for when enemy is hit
	void HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

	void SetEnemyLevel(int32 InLevel) { Level = InLevel; }

	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangedSignature OnMaxHealthChanged;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float LifeSpan = 5;

	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	TObjectPtr<AActor> CombatTarget;

protected:
	virtual void BeginPlay() override;

	virtual void InitAbilityActorInfo() override;

	virtual void InitializeDefaultAttributes() override;
	virtual void StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount) override;

	UFUNCTION(BlueprintImplementableEvent)
	void SpawnLoot();

	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "Character Class Defaults")
	int32 Level = 1;

	

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UWidgetComponent> HealthBar;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	UPROPERTY()
	TObjectPtr<ARPGAIController> RPGAIController;
};
