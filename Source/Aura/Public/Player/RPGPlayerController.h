// Copyright Kyle Murray

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RPGPlayerController.generated.h"

// Forward Declarations
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class IEnemyInterface;

/**
 * 
 */
UCLASS()
class AURA_API ARPGPlayerController : public APlayerController
{
	GENERATED_BODY()
	
private:

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> RPGContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	void Move(const FInputActionValue& InputActionValue);

	void CursorTrace();

	TScriptInterface<IEnemyInterface> LastActor;
	TScriptInterface<IEnemyInterface> ThisActor;

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

public:
	ARPGPlayerController();

	virtual void PlayerTick(float DeltaTime) override;
};
