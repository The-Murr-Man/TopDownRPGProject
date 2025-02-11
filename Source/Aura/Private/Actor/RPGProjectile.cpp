// Copyright Kyle Murray


#include "Actor/RPGProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/AudioComponent.h"
#include "Aura/Aura.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/RPGAbilitySystemLibrary.h"

// Sets default values
ARPGProjectile::ARPGProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	Sphere = CreateDefaultSubobject<USphereComponent>("Sphere");
	SetRootComponent(Sphere);

	// Only want things to overlap with this actor
	Sphere->SetCollisionObjectType(ECC_Projectile);
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	// Setting up collision channels
	
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	

	// Creating and setting up Projectile Movement Component
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement");
	ProjectileMovement->InitialSpeed = 550;
	ProjectileMovement->MaxSpeed = 550;
	ProjectileMovement->ProjectileGravityScale = 0;
}

// Called when the game starts or when spawned
void ARPGProjectile::BeginPlay()
{
	Super::BeginPlay();
	SetReplicateMovement(true);
	SetLifeSpan(LifeSpan);

	Sphere->OnComponentBeginOverlap.AddDynamic(this, &ARPGProjectile::OnSphereOverlap);

	LoopingSoundComponent =  UGameplayStatics::SpawnSoundAttached(LoopingSound, GetRootComponent());
}

void ARPGProjectile::Destroyed()
{
	if (LoopingSoundComponent)
	{
		LoopingSoundComponent->Stop();
		LoopingSoundComponent->DestroyComponent();
	}

	if (!bHit && !HasAuthority())
	{
		OnHit();
	}

	Super::Destroyed();
}

void ARPGProjectile::OnHit()
{
	UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation(), FRotator::ZeroRotator);
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ImpactEffect, GetActorLocation());
	if (LoopingSoundComponent)
	{
		LoopingSoundComponent->Stop();
		LoopingSoundComponent->DestroyComponent();
	}
	bHit = true;
}

void ARPGProjectile::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValid(DamageEffectParams.SourceAbilitySystemComponent)) return; //tested and works.

	AActor* SourceAvatarActor = DamageEffectParams.SourceAbilitySystemComponent->GetAvatarActor();

	if (SourceAvatarActor == OtherActor) return;

	if (!URPGAbilitySystemLibrary::IsNotFriend(SourceAvatarActor, OtherActor)) return;

	if (!bHit)
		OnHit();

	if (HasAuthority())
	{
		if ( UAbilitySystemComponent* TargetASC =  UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
		{
			const FVector DeathImpulse = GetActorForwardVector() * DamageEffectParams.DeathImpulseMagnitude;
			DamageEffectParams.DeathImpulse = DeathImpulse;

			const bool bKnockback = FMath::RandRange(1, 100) < DamageEffectParams.KnockbackChance;

			if (bKnockback)
			{
				FRotator Rotation = GetActorRotation();
				Rotation.Pitch = 45;

				const FVector KnockbackDirection = Rotation.Vector();
				const FVector KnockbackForce = KnockbackDirection * DamageEffectParams.KnockbackForceMagnitude;
				DamageEffectParams.KnockbackForce = KnockbackForce;
			}

			DamageEffectParams.TargetAbilitySystemComponent = TargetASC;
			URPGAbilitySystemLibrary::ApplyDamageEffect(DamageEffectParams);
		}

		Destroy();
	}

	else
		bHit = true;

}

