// Copyright Kyle Murray

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Interaction/CombatInterface.h"
#include "RPGCharacterBase.generated.h"

// Forward Declarations
class UAbilitySystemComponent;
class UAttributeSet;
class UGameplayEffect;
class UGameplayAbility;
class UAnimMontage;
class UNiagaraSystem;

UCLASS(Abstract)
class AURA_API ARPGCharacterBase : public ACharacter, public IAbilitySystemInterface, public ICombatInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ARPGCharacterBase();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UAttributeSet* GetAttributeSet() const { return AttributeSet; }

	/** Combat Interface */
	virtual UAnimMontage* GetHitReactMontage_Implementation() override { return HitReactMontage; };
	// Used on Server
	virtual void Die() override;
	virtual FVector GetCombatSocketLocation_Implementation(const FGameplayTag& MontageTag) override;
	virtual bool IsDead_Implementation() const override;
	virtual AActor* GetAvatar_Implementation() override;
	virtual TArray<FTaggedMontage> GetAttackMontages_Implementation() { return AttackMontages; }
	virtual UNiagaraSystem* GetBloodEffect_Implementation() override {return BloodEffect; }
	virtual FTaggedMontage GetTaggedMontageByTag_Implementation(const FGameplayTag& MontageTag) override;
	virtual int32 GetMinionCount_Implementation() override { return MinionCount; }
	virtual void IncrementMinionCount_Implementation(int32 Amount) override { MinionCount+= Amount; }
	/** Combat Interface */


	// Used on both server and client
	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastHandleDeath();

	UPROPERTY(EditAnywhere, Category = "Combat");
	TArray<FTaggedMontage> AttackMontages;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void InitAbilityActorInfo();

	virtual void InitializeDefaultAttributes();
	void ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass,float Level);

	void AddCharacterAbilities();

	// Swaps Character and weapon materials to dissolve mat
	void Dissolve();
	
	UFUNCTION(BlueprintImplementableEvent)
	void StartDissolveTimeline(UMaterialInstanceDynamic* DynamicMaterialInstance);

	UFUNCTION(BlueprintImplementableEvent)
	void StartWeaponDissolveTimeline(UMaterialInstanceDynamic* DynamicMaterialInstance);

	
	// Mesh for the characters weapon
	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "Combat");
	TObjectPtr<USkeletalMeshComponent> WeaponMesh;

	// Weapon Socket used for projectile spawning
	UPROPERTY(EditAnywhere, Category = "Combat");
	FName WeaponTipSocketName;

	UPROPERTY(EditAnywhere, Category = "Combat");
	FName LeftHandSocketName;

	UPROPERTY(EditAnywhere, Category = "Combat");
	FName RightHandSocketName;

	UPROPERTY(EditAnywhere, Category = "Combat");
	FName TailSocketName;

	bool bDead = false;

	/* Minions */
	int32 MinionCount = 0;

	// Dissolve Effects
	UPROPERTY(BlueprintReadOnly, EditAnywhere);
	TObjectPtr<UMaterialInstance> DissolveMaterialInstance;

	UPROPERTY(BlueprintReadOnly, EditAnywhere);
	TObjectPtr<UMaterialInstance> WeaponDissolveMaterialInstance;

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Attributes);
	TSubclassOf<UGameplayEffect> DefaultPrimaryAttributes;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Attributes);
	TSubclassOf<UGameplayEffect> DefaultSecondaryAttributes;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = Attributes);
	TSubclassOf<UGameplayEffect> DefaultVitalAttributes;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Combat");
	UNiagaraSystem* BloodEffect;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Combat");
	TObjectPtr<USoundBase> DeathSound;

private:

	UPROPERTY(EditAnywhere, Category = "Abilities");
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;

	UPROPERTY(EditAnywhere, Category = "Combat");
	TObjectPtr<UAnimMontage> HitReactMontage;
};
