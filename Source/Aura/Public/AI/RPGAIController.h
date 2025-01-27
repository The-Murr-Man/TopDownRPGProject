// Copyright Kyle Murray

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "RPGAIController.generated.h"

class UBehaviorTreeComponent;
/**
 * 
 */
UCLASS()
class AURA_API ARPGAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	ARPGAIController();

protected:
	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> BehaviorTreeComponent;

private:
};
