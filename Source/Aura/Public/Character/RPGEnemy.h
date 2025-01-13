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
	
protected:
	virtual void BeginPlay() override;

public:
	ARPGEnemy();

	// Enemy Interface functions
	virtual void HighlightActor() override;

	virtual void UnHighlightActor() override;

};
