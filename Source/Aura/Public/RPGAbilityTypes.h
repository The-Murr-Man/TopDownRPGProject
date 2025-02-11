// Copyright Kyle Murray

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "RPGAbilityTypes.generated.h"

class UGameplayEffect;

USTRUCT(BlueprintType)
struct FDamageEffectParams
{
	GENERATED_BODY()

	FDamageEffectParams() {};

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UObject> WorldContextObject = nullptr;

	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<UGameplayEffect> DamageGameplayEffectClass = nullptr;
	
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UAbilitySystemComponent> SourceAbilitySystemComponent;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UAbilitySystemComponent> TargetAbilitySystemComponent;

	UPROPERTY(BlueprintReadWrite)
	float BaseDamage = 0;

	UPROPERTY(BlueprintReadWrite)
	float AbilityLevel = 1;

	UPROPERTY(BlueprintReadWrite)
	FGameplayTag DamageType = FGameplayTag();

	UPROPERTY(BlueprintReadWrite)
	float DebuffChance = 0;

	UPROPERTY(BlueprintReadWrite)
	float DebuffDamage = 0;

	UPROPERTY(BlueprintReadWrite)
	float DebuffFrequency = 0;

	UPROPERTY(BlueprintReadWrite)
	float DebuffDuration = 0;

	UPROPERTY(BlueprintReadWrite)
	float DeathImpulseMagnitude = 0;

	UPROPERTY(BlueprintReadWrite)
	FVector DeathImpulse = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite)
	float KnockbackForceMagnitude = 0;

	UPROPERTY(BlueprintReadWrite)
	FVector KnockbackForce = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite)
	float KnockbackChance = 0;
};

/// <summary>
/// Everything in this struct is REQUIRED for making a Custom Gameplay Effect Context
/// </summary>
USTRUCT(BlueprintType)
struct FRPGGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

public: // Redundant but I like it this way

	/** Returns the actual struct used for serialization, subclasses must override this! */
	virtual UScriptStruct* GetScriptStruct() const { return StaticStruct(); }
	
	/** Creates a copy of this context, used to duplicate for later modifications */
	virtual FRPGGameplayEffectContext* Duplicate() const
	{
		FRPGGameplayEffectContext* NewContext = new FRPGGameplayEffectContext();
		*NewContext = *this;
		if (GetHitResult())
		{
			// Does a deep copy of the hit result
			NewContext->AddHitResult(*GetHitResult(), true);
		}
		return NewContext;
	}

	/** Custom serialization, subclasses must override this */
	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	bool IsCriticalHit() const { return bIsCriticalHit; }
	bool IsBlockedHit() const { return bIsBlockedlHit; }
	bool IsSuccessfulDebuff() const { return bIsSuccessfulDebuff; }

	float GetDebuffDamage() const { return DebuffDamage; }
	float GetDebuffDuration() const { return DebuffDuration; }
	float GetDebuffFrequency() const { return DebuffFrequency; }

	TSharedPtr<FGameplayTag> GetDamageType() const { return DamageType; }

	FVector GetDeathImpulse() const { return DeathImpulse; }
	FVector GetKnockbackForce() const { return KnockbackForce; }

	void SetIsCriticalHit(bool bInIsCriticalHit) { bIsCriticalHit = bInIsCriticalHit; }
	void SetIsBlockedlHit(bool bInIsBlockedHit) { bIsBlockedlHit = bInIsBlockedHit; }
	void SetIsSuccessfulDebuff(bool bInIsSuccessfulDebuff) { bIsSuccessfulDebuff = bInIsSuccessfulDebuff; }

	void SetDebuffDamage(float InDebuffDamage) { DebuffDamage = InDebuffDamage; }
	void SetDebuffDuration(float InDebuffDuration) { DebuffDuration = InDebuffDuration; }
	void SetDebuffFrequency(float InDebuffFrequency) { DebuffFrequency = InDebuffFrequency; }

	void SetDamageType(TSharedPtr<FGameplayTag> InDamageType) { DamageType = InDamageType; }

	void SetDeathImpulse(const FVector InDeathImpulse) { DeathImpulse = InDeathImpulse; }
	void SetKnockbackForce(FVector InKnockbackForce) { KnockbackForce = InKnockbackForce; }

protected:

	UPROPERTY()
	bool bIsBlockedlHit = false;

	UPROPERTY()
	bool bIsCriticalHit = false;

	UPROPERTY()
	bool bIsSuccessfulDebuff = false;

	UPROPERTY()
	float DebuffDamage = 0;

	UPROPERTY()
	float DebuffDuration = 0;

	UPROPERTY()
	float DebuffFrequency = 0;

	UPROPERTY()
	FVector DeathImpulse = FVector::ZeroVector;

	UPROPERTY()
	FVector KnockbackForce = FVector::ZeroVector;

	TSharedPtr<FGameplayTag> DamageType;
};

template<>
struct TStructOpsTypeTraits<FRPGGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FRPGGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true		// Necessary so that TSharedPtr<FHitResult> Data is copied around
	};
};