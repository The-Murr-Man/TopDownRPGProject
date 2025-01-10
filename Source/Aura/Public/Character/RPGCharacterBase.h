// Copyright Kyle Murray

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RPGCharacterBase.generated.h"

UCLASS(Abstract)
class AURA_API ARPGCharacterBase : public ACharacter
{
	GENERATED_BODY()

private:


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Mesh for the characters weapon
	UPROPERTY(EditAnywhere, Category = "Combat");
	TObjectPtr<USkeletalMeshComponent> WeaponMesh;

public:
	// Sets default values for this character's properties
	ARPGCharacterBase();

};
