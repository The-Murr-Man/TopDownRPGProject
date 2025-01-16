// Copyright Kyle Murray

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "RPGCharacterBase.generated.h"

// Forward Declarations
class UAbilitySystemComponent;
class UAttributeSet;

UCLASS(Abstract)
class AURA_API ARPGCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ARPGCharacterBase();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UAttributeSet* GetAttributeSet() const { return AttributeSet; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Mesh for the characters weapon
	UPROPERTY(EditAnywhere, Category = "Combat");
	TObjectPtr<USkeletalMeshComponent> WeaponMesh;

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

	virtual void InitAbilityActorInfo();
};
