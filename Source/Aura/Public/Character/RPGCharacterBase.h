// Copyright Kyle Murray

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "Interaction/CombatInterface.h"
#include "RPGCharacterBase.generated.h"

// Forward Declarations
class UAbilitySystemComponent;
class UAttributeSet;
class UGameplayEffect;
class UGameplayAbility;
class UAnimMontage;
class UNiagaraSystem;
class UDebuffNiagaraComponent;
class UPassiveNiagaraComponent;


UCLASS(Abstract)
class AURA_API ARPGCharacterBase : public ACharacter, public IAbilitySystemInterface, public ICombatInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ARPGCharacterBase();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	UAttributeSet* GetAttributeSet() const { return AttributeSet; }

	/** Combat Interface */
	virtual UAnimMontage* GetHitReactMontage_Implementation() override { return HitReactMontage; };
	// Used on Server
	virtual void Die(const FVector& DeathImpulse) override;
	virtual FVector GetCombatSocketLocation_Implementation(const FGameplayTag& MontageTag) override;
	virtual bool IsDead_Implementation() const override;
	virtual AActor* GetAvatar_Implementation() override;
	virtual TArray<FTaggedMontage> GetAttackMontages_Implementation() { return AttackMontages; }
	virtual UNiagaraSystem* GetBloodEffect_Implementation() override {return BloodEffect; }
	virtual FTaggedMontage GetTaggedMontageByTag_Implementation(const FGameplayTag& MontageTag) override;
	virtual int32 GetMinionCount_Implementation() override { return MinionCount; }
	virtual void IncrementMinionCount_Implementation(int32 Amount) override { MinionCount+= Amount; }
	virtual ECharacterClass GetCharacterClass_Implementation() override { return CharacterClass; }
	virtual FOnASCRegistered& GetOnASCRegisteredDelegate() override { return OnASCRegistered; }
	virtual FOnDeath& GetOnDeathDelegate() override { return OnDeath; }
	virtual USkeletalMeshComponent* GetWeaponMesh_Implementation() override { return WeaponMesh; }
	virtual void SetIsBeingShocked_Implementation(bool bInShock) override { bIsBeingShocked = bInShock; }
	virtual bool IsBeingShocked_Implementation() const override { return bIsBeingShocked;}
	virtual FOnDamageSignature& GetOnDamageDelegate() override { return OnDamageDelegate; }
	/** Combat Interface */

	FOnASCRegistered OnASCRegistered;
	FOnDeath OnDeath;
	
	FOnDamageSignature OnDamageDelegate;
	// Used on both server and client
	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastHandleDeath(const FVector& DeathImpulse);

	UPROPERTY(EditAnywhere, Category = "Combat");
	TArray<FTaggedMontage> AttackMontages;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bHitReacting = false;

	virtual void StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float BaseMaxWalkSpeed = 600;

	// Debuff Bools
	UPROPERTY(ReplicatedUsing = OnRep_Stunned, BlueprintReadOnly, Category = "Combat|Debuffs");
	bool bIsStunned = false;

	UPROPERTY(ReplicatedUsing = OnRep_Burned, BlueprintReadOnly, Category = "Combat|Debuffs");
	bool bIsBurned = false;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Combat|Debuffs");
	bool bIsBeingShocked = false;

	UFUNCTION()
	virtual void OnRep_Stunned();

	UFUNCTION()
	virtual void OnRep_Burned();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Class Defaults")
	ECharacterClass CharacterClass = ECharacterClass::Warrior;

	/* Minions */
	int32 MinionCount = 0;

	// Dissolve Effects
	UPROPERTY(BlueprintReadOnly, EditAnywhere);
	TObjectPtr<UMaterialInstance> DissolveMaterialInstance;

	UPROPERTY(BlueprintReadOnly, EditAnywhere);
	TObjectPtr<UMaterialInstance> WeaponDissolveMaterialInstance;

	// GAS
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
	//

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Combat");
	UNiagaraSystem* BloodEffect;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Combat");
	TObjectPtr<USoundBase> DeathSound;

	// Debuff Components
	UPROPERTY(VisibleAnywhere, Category = "Combat|Debuff");
	TObjectPtr<UDebuffNiagaraComponent> BurnDebuffComponent;

	UPROPERTY(VisibleAnywhere, Category = "Combat|Debuff");
	TObjectPtr<UDebuffNiagaraComponent> StunDebuffComponent;
	
private:

	UPROPERTY(EditAnywhere, Category = "Abilities");
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;

	UPROPERTY(EditAnywhere, Category = "Abilities");
	TArray<TSubclassOf<UGameplayAbility>> StartupPassiveAbilities;

	UPROPERTY(EditAnywhere, Category = "Combat");
	TObjectPtr<UAnimMontage> HitReactMontage;

	//Passive Niagara Components
	UPROPERTY(VisibleAnywhere, Category = "Combat|Passives");
	TObjectPtr<UPassiveNiagaraComponent> HaloOfProtectionNiagaraComponent;

	UPROPERTY(VisibleAnywhere, Category = "Combat|Passives");
	TObjectPtr<UPassiveNiagaraComponent> LifeSiphonNiagaraComponent;

	UPROPERTY(VisibleAnywhere, Category = "Combat|Passives");
	TObjectPtr<UPassiveNiagaraComponent> ManaSiphonNiagaraComponent;

	UPROPERTY(VisibleAnywhere, Category = "Combat|Passives");
	TObjectPtr<USceneComponent> EffectAttachComponent;
};
