// Copyright Kyle Murray

#pragma once

#include "CoreMinimal.h"
#include "Character/RPGCharacterBase.h"
#include "Interaction/EnemyInterface.h"
#include "RPGEnemy.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API ARPGEnemy : public ARPGCharacterBase, public IEnemyInterface
{
	GENERATED_BODY()
	
public:
	ARPGEnemy();

	// Enemy Interface functions
	virtual void HighlightActor() override;
	virtual void UnHighlightActor() override;
	//

	// Combat Interface
	virtual int32 GetPlayerLevel() override;
	//

protected:
	virtual void BeginPlay() override;

	virtual void InitAbilityActorInfo() override;

	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "Character Class Defaults")
	int32 Level = 1;

};
