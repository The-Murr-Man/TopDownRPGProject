// Copyright Kyle Murray

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "TargetDataUnderMouse.generated.h"

class AbilitySystemComponent;

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMouseTargetDataSigniture, const FGameplayAbilityTargetDataHandle&, DataHandle);

/**
 * 
 */
UCLASS()
class AURA_API UTargetDataUnderMouse : public UAbilityTask
{
	GENERATED_BODY()

public:

	/// <summary>
	/// Ability Task that get the data of the target under the mouse cursor,
	/// Owning Ability is hidden and defaults to self
	/// </summary>
	/// <param name="OwningAbility"></param>
	/// <returns></returns>
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (DisplayName = "TargetDataUnderMouse", HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = true))
	static UTargetDataUnderMouse* CreateTargetDataUnderMouse(UGameplayAbility* OwningAbility);

	UPROPERTY(BlueprintAssignable)
	FMouseTargetDataSigniture ValidData;

private:
	virtual void Activate() override;

	void SendMouseCursorData();

	void OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& DataHandle, FGameplayTag ActivationTag);
};
