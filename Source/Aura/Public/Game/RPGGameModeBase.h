// Copyright Kyle Murray

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RPGGameModeBase.generated.h"

class UCharacterClassInfo;
class UAbilityInfo;
/**
 * 
 */
UCLASS()
class AURA_API ARPGGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditDefaultsOnly, Category = "Character Class Defaults")
	TObjectPtr<UCharacterClassInfo> CharacterClassInfo;

	UPROPERTY(EditDefaultsOnly, Category = "Ability Info")
	TObjectPtr<UAbilityInfo> AbilityInfo;
};
