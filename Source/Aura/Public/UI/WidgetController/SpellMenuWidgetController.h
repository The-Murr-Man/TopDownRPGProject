// Copyright Kyle Murray

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/RPGWidgetController.h"
#include "GameplayTagContainer.h"
#include "RPGGameplayTags.h"
#include "SpellMenuWidgetController.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnSpellGlobeSelectSignature, bool, bSpendPointsButtonEnabled, bool, bEqiupButtonEnabled, FString, DescriptionString, FString, NexLevelDescriptionString);

struct FSelectedAbility
{
	FGameplayTag Ability = FGameplayTag();
	FGameplayTag Status = FGameplayTag();
};

/**
 * 
 */
UCLASS()
class AURA_API USpellMenuWidgetController : public URPGWidgetController
{
	GENERATED_BODY()
	
public:
	virtual void BroadcastInitialValues() override;
	virtual void BindCallbacksToDependencies() override;

	UFUNCTION(BlueprintCallable)
	void SpellGlobeSelected(const FGameplayTag& AbilityTag);

	UFUNCTION(BlueprintCallable)
	void SpendPointButtonPressed();

	UPROPERTY(BlueprintAssignable)
	FOnPlayerStatChangedSignature SpellPointsChangedDelegate;

	UPROPERTY(BlueprintAssignable)
	FOnSpellGlobeSelectSignature SpellGlobeSelectedDelegate;

private:
	static void ShouldEnableButtons(const FGameplayTag& AbilityStatus, int32 SpellPoints, bool& bShouldEnableSpendPointsButton, bool& bShouldEnableEquipButton);

	FSelectedAbility SelectedAbility = { FGameplayTag::EmptyTag,FRPGGameplayTags::Get().Abilities_Status_Locked };

	int32 CurrentSpellPoints = 0;
};
