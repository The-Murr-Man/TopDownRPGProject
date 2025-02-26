// Copyright Kyle Murray

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RPGEnemySpawnVolume.generated.h"

class UBoxComponent;
class ARPGEnemySpawnPoint;
UCLASS()
class AURA_API ARPGEnemySpawnVolume : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARPGEnemySpawnVolume();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(EditAnywhere)
	TArray<ARPGEnemySpawnPoint*> SpawnPoints;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> Box;
};
