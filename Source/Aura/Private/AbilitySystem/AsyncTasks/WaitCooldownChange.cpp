// Copyright Kyle Murray


#include "AbilitySystem/AsyncTasks/WaitCooldownChange.h"
#include "AbilitySystemComponent.h"

/// <summary>
/// 
/// </summary>
/// <param name="AbilitySystemComponent"></param>
/// <param name="InCooldownTag"></param>
/// <returns></returns>
UWaitCooldownChange* UWaitCooldownChange::WaitForCooldownChange(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayTag& InCooldownTag)
{
    UWaitCooldownChange* WaitCooldownChange = NewObject<UWaitCooldownChange>();
    WaitCooldownChange->ASC = AbilitySystemComponent;
    WaitCooldownChange->CooldownTag = InCooldownTag;

    if (!IsValid(AbilitySystemComponent) || !InCooldownTag.IsValid())
    {
        WaitCooldownChange->EndTask();
        return nullptr;
    }
   
    // To know when a cooldown has been ended (CooldownTag has been removed)
    AbilitySystemComponent->RegisterGameplayTagEvent(InCooldownTag, EGameplayTagEventType::NewOrRemoved).AddUObject(WaitCooldownChange, &UWaitCooldownChange::CooldownTagChanged);

    // To know when a cooldown effect has been applied
    AbilitySystemComponent->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(WaitCooldownChange, &UWaitCooldownChange::OnActiveEffectAdded);

    return WaitCooldownChange;
}

/// <summary>
/// Removes the task from the ability system component
/// </summary>
void UWaitCooldownChange::EndTask()
{
    if (!IsValid(ASC)) return;
    ASC->RegisterGameplayTagEvent(CooldownTag, EGameplayTagEventType::NewOrRemoved).RemoveAll(this);

    SetReadyToDestroy();
    MarkAsGarbage();
}

/// <summary>
/// 
/// </summary>
/// <param name="InCooldownTag"></param>
/// <param name="NewCount"></param>
void UWaitCooldownChange::CooldownTagChanged(const FGameplayTag InCooldownTag, int32 NewCount)
{
    if (NewCount <= 0)
    {
        CooldownEnd.Broadcast(0.f);
    }


}

/// <summary>
/// Gets all the tags to see if we have applied a CooldownTag, then gets the time remaining on the cooldown effect
/// </summary>
/// <param name="TargetASC"></param>
/// <param name="SpecApplied"></param>
/// <param name="ActiveEffectHandle"></param>
void UWaitCooldownChange::OnActiveEffectAdded(UAbilitySystemComponent* TargetASC, const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveEffectHandle)
{
    FGameplayTagContainer AssetTags;
    SpecApplied.GetAllAssetTags(AssetTags);

    FGameplayTagContainer GrantedTags;
    SpecApplied.GetAllGrantedTags(GrantedTags);

    // Checking if tag applied is a cooldown tag
    if (AssetTags.HasTagExact(CooldownTag) || GrantedTags.HasTagExact(CooldownTag))
    {
        // Creates a query for our cooldown tag
        FGameplayEffectQuery GameplayEffectQuery = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(CooldownTag.GetSingleTagContainer());

        // Gets the time remaining on our cooldows
        TArray<float> TimesRemaining = ASC->GetActiveEffectsTimeRemaining(GameplayEffectQuery);

        // Sets TimeRemaining to the highest time in TimesRemaining
        const float TimeRemaining = FMath::Max(TimesRemaining);

        CooldownStart.Broadcast(TimeRemaining);
    }
}
