// Copyright Kyle Murray


#include "AbilitySystem/Abilities/RPGProjectileSpell.h"
#include "Actor/RPGProjectile.h"
#include "Interaction/CombatInterface.h"

void URPGProjectileSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	const bool bIsServer = (HasAuthority(&ActivationInfo));
	
	if (!bIsServer) return;

	ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetAvatarActorFromActorInfo());

	if (CombatInterface)
	{
		const FVector SocketLocation =  CombatInterface->GetCombatSocketLocation();

		FTransform SpawnTransform;
		SpawnTransform.SetLocation(SocketLocation);

		// TODO: Set the projectile Rotation

		AActor* OwningActor = GetOwningActorFromActorInfo();

		ARPGProjectile* Projectile = GetWorld()->SpawnActorDeferred<ARPGProjectile>(ProjectileClass, SpawnTransform, OwningActor, Cast<APawn>(OwningActor), ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		
		// TODO: Give Projectile a Gameplay Effect Spec for causing damage

		Projectile->FinishSpawning(SpawnTransform);
	}
}
