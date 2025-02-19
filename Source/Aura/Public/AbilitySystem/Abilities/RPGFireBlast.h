// Copyright Kyle Murray

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/RPGDamageGameplayAbility.h"
#include "RPGFireBlast.generated.h"

class ARPGFireBall;
/**
 * 
 */
UCLASS()
class AURA_API URPGFireBlast : public URPGDamageGameplayAbility
{
	GENERATED_BODY()
	
public:
	virtual FString GetDescription(int32 Level) override;
	virtual FString GetNextLevelDescription(int32 Level) override;

	UFUNCTION(BlueprintCallable)
	TArray<ARPGFireBall*> SpawnFireBalls();

protected:

	UPROPERTY(EditDefaultsOnly, Category = "FireBlast")
	int32 NumFireBalls = 12;

private:

	UPROPERTY(EditDefaultsOnly, Category = "FireBlast")
	TSubclassOf<ARPGFireBall> FireBallClass;
};
