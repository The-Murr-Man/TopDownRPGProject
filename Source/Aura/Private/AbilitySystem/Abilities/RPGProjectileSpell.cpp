// Copyright Kyle Murray


#include "AbilitySystem/Abilities/RPGProjectileSpell.h"
#include "Actor/RPGProjectile.h"
#include "Interaction/CombatInterface.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "RPGGameplayTags.h"

void URPGProjectileSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void URPGProjectileSpell::SpawnProjectile(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag, bool bOverridePitch, float PitchOverride)
{
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();

	if (!bIsServer) return;

	ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetAvatarActorFromActorInfo());

	if (CombatInterface)
	{
		const FVector SocketLocation = CombatInterface->Execute_GetCombatSocketLocation(GetAvatarActorFromActorInfo(),SocketTag);
		FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation();

		if (bOverridePitch) Rotation.Pitch = PitchOverride;

		FTransform SpawnTransform;
		SpawnTransform.SetLocation(SocketLocation);

		// TODO: Set the projectile Rotation
		SpawnTransform.SetRotation(Rotation.Quaternion());

		
		AActor* OwningActor = GetOwningActorFromActorInfo();

		ARPGProjectile* Projectile = GetWorld()->SpawnActorDeferred<ARPGProjectile>(ProjectileClass, SpawnTransform, OwningActor, Cast<APawn>(OwningActor), ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		UAbilitySystemComponent* SourceASC =  UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo());

		FGameplayEffectContextHandle EffectContextHandle = SourceASC->MakeEffectContext();
		FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), EffectContextHandle);
		EffectContextHandle.SetAbility(this);
		EffectContextHandle.AddSourceObject(Projectile);

		// Gets the GameplayTags for our singleton
		FRPGGameplayTags GameplayTags = FRPGGameplayTags::Get();

		for (auto& Pair : DamageTypes)
		{
			// Gets Damage at the abilities level from curve;
			const float ScaledDamage = Pair.Value.GetValueAtLevel(GetAbilityLevel());
			UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, Pair.Key, ScaledDamage);
		}
		
		Projectile->DamageEffectSpecHandle = SpecHandle;

		Projectile->FinishSpawning(SpawnTransform);
	}
}
