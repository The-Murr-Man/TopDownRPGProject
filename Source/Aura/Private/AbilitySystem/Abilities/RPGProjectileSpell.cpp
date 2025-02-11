// Copyright Kyle Murray


#include "AbilitySystem/Abilities/RPGProjectileSpell.h"
#include "Actor/RPGProjectile.h"
#include "Interaction/CombatInterface.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "RPGGameplayTags.h"
#include "RPGAbilityTypes.h"

void URPGProjectileSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void URPGProjectileSpell::SpawnProjectile(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag, bool bOverridePitch, float PitchOverride)
{
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();

	if (!bIsServer) return;

	ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetAvatarActorFromActorInfo());

	if (!CombatInterface) return;

	const FVector SocketLocation = CombatInterface->Execute_GetCombatSocketLocation(GetAvatarActorFromActorInfo(),SocketTag);
	FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation();

	if (bOverridePitch) Rotation.Pitch = PitchOverride;

	FTransform SpawnTransform;
	SpawnTransform.SetLocation(SocketLocation);

	SpawnTransform.SetRotation(Rotation.Quaternion());

	AActor* OwningActor = GetOwningActorFromActorInfo();

	ARPGProjectile* Projectile = GetWorld()->SpawnActorDeferred<ARPGProjectile>(ProjectileClass, SpawnTransform, OwningActor, Cast<APawn>(OwningActor), ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	// Damage Effect Params
	Projectile->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();


	Projectile->FinishSpawning(SpawnTransform);
	
}
