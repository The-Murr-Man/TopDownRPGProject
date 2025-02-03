// Copyright Kyle Murray


#include "UI/WidgetController/OverlayWidgetController.h"
#include "AbilitySystem/RPGAttributeSet.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "Player/RPGPlayerState.h"
#include "AbilitySystem/Data/LevelUpInfo.h"

/// <summary>
/// 
/// </summary>
void UOverlayWidgetController::BroadcastInitialValues()
{
	const URPGAttributeSet* RPGAttributeSet = CastChecked<URPGAttributeSet>(AttributeSet);

	
	OnHealthChanged.Broadcast(RPGAttributeSet->GetHealth());
	OnMaxHealthChanged.Broadcast(RPGAttributeSet->GetMaxHealth());

	OnManaChanged.Broadcast(RPGAttributeSet->GetMana());
	OnMaxManaChanged.Broadcast(RPGAttributeSet->GetMaxMana());
}

/// <summary>
/// 
/// </summary>
void UOverlayWidgetController::BindCallbacksToDependencies()
{
	ARPGPlayerState* RPGPlayerState = CastChecked<ARPGPlayerState>(PlayerState);

	// Callback for changing xp
	RPGPlayerState->OnXPChangedDelegate.AddUObject(this, &UOverlayWidgetController::OnXPChanged);

	const URPGAttributeSet* RPGAttributeSet = CastChecked<URPGAttributeSet>(AttributeSet);
	
	BindAttributeChange(RPGAttributeSet->GetHealthAttribute(), OnHealthChanged);
	BindAttributeChange(RPGAttributeSet->GetMaxHealthAttribute(), OnMaxHealthChanged);
	BindAttributeChange(RPGAttributeSet->GetManaAttribute(), OnManaChanged);
	BindAttributeChange(RPGAttributeSet->GetMaxManaAttribute(), OnMaxManaChanged);

	if (URPGAbilitySystemComponent* RPGASC = Cast<URPGAbilitySystemComponent>(AbilitySystemComponent))
	{
		if (RPGASC->bStartupAbilitiesGiven)
			OnInitializeStartupAbilities(RPGASC);
		else
			RPGASC->AbilitiesGivenDelegate.AddUObject(this,&UOverlayWidgetController::OnInitializeStartupAbilities);

		// Using a lamda function to loop through all tags on the ability system component
		RPGASC->EffectAssetTags.AddLambda(
			[this](const FGameplayTagContainer& AssetTags)
			{
				for (const FGameplayTag& Tag : AssetTags)
				{
					FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(FName("Message"));
					if (!Tag.MatchesTag(MessageTag)) return;

					FUIWidgetRow* Row = GetDataTableRowByTag<FUIWidgetRow>(MessageWidgetDataTable, Tag);
					MessegeWidgetRowDelegate.Broadcast(*Row);
				}
			}
		);
	}
	
}

/// <summary>
/// 
/// </summary>
/// <param name="Attribute"></param>
/// <param name="AttributeData"></param>
void UOverlayWidgetController::BindAttributeChange(FGameplayAttribute Attribute, FOnAttributeChangedSignature& AttributeData)
{
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Attribute).AddLambda(
		[this, &AttributeData](const FOnAttributeChangeData& Data)
		{
			AttributeData.Broadcast(Data.NewValue);
		}
	);
}

/// <summary>
/// 
/// </summary>
void UOverlayWidgetController::OnInitializeStartupAbilities(URPGAbilitySystemComponent* RPGAbilitySystemComponent)
{
	if (!RPGAbilitySystemComponent->bStartupAbilitiesGiven) return;

	FForEachAbility BroadcastDelegate;
	BroadcastDelegate.BindLambda([this, RPGAbilitySystemComponent] (const FGameplayAbilitySpec& AbilitySpec)
	{
		FRPGAbilityInfo Info =  AbilityInfo->FindAbilityInfoForTag(RPGAbilitySystemComponent->GetAbilityTagFromSpec(AbilitySpec));
		Info.InputTag = RPGAbilitySystemComponent->GetInputTagFromSpec(AbilitySpec);
		AbilityInfoDelegate.Broadcast(Info);
	});

	RPGAbilitySystemComponent->ForEachAbility(BroadcastDelegate);
}

/// <summary>
/// 
/// </summary>
/// <param name="NewXP"></param>
void UOverlayWidgetController::OnXPChanged(int32 NewXP)
{
	ARPGPlayerState* RPGPlayerState = CastChecked<ARPGPlayerState>(PlayerState);

	ULevelUpInfo* LevelUpInfo = RPGPlayerState->LevelUpInfo;

	checkf(LevelUpInfo, TEXT("Unable to find LevelUpInfo, Please fill our RPGPlayerState Blueprint"));

	int32 Level = LevelUpInfo->FindLevelForXP(NewXP);
	int32 MaxLevel = LevelUpInfo->LevelUpInformation.Num();

	if (Level <= MaxLevel && Level > 0)
	{
		int32 LevelUpRequirment = LevelUpInfo->LevelUpInformation[Level].LevelUpRequirment;
		int32 PreviousLevelUpRequirment = LevelUpInfo->LevelUpInformation[Level - 1].LevelUpRequirment;

		int32 DeltaLevelUpRequirment = LevelUpRequirment - PreviousLevelUpRequirment;
		int32 XPForThisLevel = NewXP - PreviousLevelUpRequirment;

		float XPBarPercent = static_cast<float>(XPForThisLevel) / static_cast<float>(DeltaLevelUpRequirment);

		OnXPPercentChangedDelegate.Broadcast(XPBarPercent);
	}
}
