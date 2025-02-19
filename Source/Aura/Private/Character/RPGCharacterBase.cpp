// Copyright Kyle Murray


#include "Character/RPGCharacterBase.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "Aura/Aura.h"
#include "RPGGameplayTags.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystem/Debuff/DebuffNiagaraComponent.h"
#include "AbilitySystem/Passive/PassiveNiagaraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ARPGCharacterBase::ARPGCharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Setting up Burn Debuff Comp
	BurnDebuffComponent = CreateDefaultSubobject<UDebuffNiagaraComponent>(TEXT("BurnDebuffComponent"));
	BurnDebuffComponent->SetupAttachment(GetRootComponent());
	BurnDebuffComponent->DebuffTag = FRPGGameplayTags::Get().Debuff_Burn;

	// Setting up Stun Debuff Comp
	StunDebuffComponent = CreateDefaultSubobject<UDebuffNiagaraComponent>(TEXT("StunDebuffComponent"));
	StunDebuffComponent->SetupAttachment(GetRootComponent());
	StunDebuffComponent->DebuffTag = FRPGGameplayTags::Get().Debuff_Stun;

	// Setting up Capsule Comp
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetGenerateOverlapEvents(false);
	GetCapsuleComponent()->SetReceivesDecals(false);

	// Setting up Mesh Comp
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Projectile, ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);
	GetMesh()->SetReceivesDecals(false);

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon Mesh");
	WeaponMesh->SetupAttachment(GetMesh(), FName("WeaponHandSocket"));
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetReceivesDecals(false);

	// Component all passive effects will attach to
	EffectAttachComponent = CreateDefaultSubobject<USceneComponent>(TEXT("EffectAttachPoint"));
	EffectAttachComponent->SetupAttachment(GetRootComponent());
	EffectAttachComponent->SetUsingAbsoluteRotation(true);
	EffectAttachComponent->SetWorldRotation(FRotator::ZeroRotator);

	// Setting up Halo Of Protection Passive Comp
	HaloOfProtectionNiagaraComponent = CreateDefaultSubobject<UPassiveNiagaraComponent>(TEXT("HaloOfProtectionComp"));
	HaloOfProtectionNiagaraComponent->SetupAttachment(EffectAttachComponent);

	// Setting up LifeSiphon Passive Comp
	LifeSiphonNiagaraComponent = CreateDefaultSubobject<UPassiveNiagaraComponent>(TEXT("LifeSiphonComp"));
	LifeSiphonNiagaraComponent->SetupAttachment(EffectAttachComponent);

	// Setting up ManaSiphon Passive Comp
	ManaSiphonNiagaraComponent = CreateDefaultSubobject<UPassiveNiagaraComponent>(TEXT("ManaSiphonComp"));
	ManaSiphonNiagaraComponent->SetupAttachment(EffectAttachComponent);
}

// Add variables that you want to be replicated here
void ARPGCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(ARPGCharacterBase, bIsStunned);
	DOREPLIFETIME(ARPGCharacterBase, bIsBurned);
	DOREPLIFETIME(ARPGCharacterBase, bIsBeingShocked);
}

// Called when the game starts or when spawned
void ARPGCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

/// <summary>
/// 
/// </summary>
void ARPGCharacterBase::InitializeDefaultAttributes()
{
	ApplyEffectToSelf(DefaultPrimaryAttributes, 1);
	ApplyEffectToSelf(DefaultSecondaryAttributes, 1);
	ApplyEffectToSelf(DefaultVitalAttributes, 1);
}

/// <summary>
/// 
/// </summary>
/// <param name="GameplayEffectClass"></param>
/// <param name="Level"></param>
void ARPGCharacterBase::ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level)
{
	if (!GetAbilitySystemComponent()) return;
	if (!GameplayEffectClass) return;

	// How to make a context Handle
	FGameplayEffectContextHandle ContextHandle = GetAbilitySystemComponent()->MakeEffectContext();
	ContextHandle.AddSourceObject(this);

	// How to make a spec Handle
	FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(GameplayEffectClass, Level, ContextHandle);
	GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), GetAbilitySystemComponent());
}

/// <summary>
/// 
/// </summary>
void ARPGCharacterBase::AddCharacterAbilities()
{
	URPGAbilitySystemComponent* RPGASC = CastChecked<URPGAbilitySystemComponent>(AbilitySystemComponent);
	if (!HasAuthority()) return;

	RPGASC->AddCharacterAbilities(StartupAbilities);
	RPGASC->AddCharacterPassiveAbilities(StartupPassiveAbilities);
}

/// <summary>
/// 
/// </summary>
/// <param name="CallbackTag"></param>
/// <param name="NewCount"></param>
void ARPGCharacterBase::StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	bIsStunned = NewCount > 0;
	GetCharacterMovement()->MaxWalkSpeed = bIsStunned ? 0 : BaseMaxWalkSpeed;
}

/// <summary>
/// 
/// </summary>
void ARPGCharacterBase::Dissolve()
{
	if (!DissolveMaterialInstance && !WeaponDissolveMaterialInstance) return;

	// Dissolve for Character Mesh
	UMaterialInstanceDynamic* DynamicMatInst = UMaterialInstanceDynamic::Create(DissolveMaterialInstance,this);
	GetMesh()->SetMaterial(0, DynamicMatInst);

	StartDissolveTimeline(DynamicMatInst);

	// Dissolve for Weapon Mesh
	UMaterialInstanceDynamic* WeaponDynamicMatInst = UMaterialInstanceDynamic::Create(WeaponDissolveMaterialInstance, this);
	WeaponMesh->SetMaterial(0, WeaponDynamicMatInst);

	StartWeaponDissolveTimeline(WeaponDynamicMatInst);
}

void ARPGCharacterBase::OnRep_Stunned()
{

}

void ARPGCharacterBase::OnRep_Burned()
{
}

/// <summary>
/// 
/// </summary>
void ARPGCharacterBase::InitAbilityActorInfo()
{
}

/// <summary>
/// 
/// </summary>
/// <param name="DeathImpulse"></param>
void ARPGCharacterBase::Die(const FVector& DeathImpulse)
{
	// Detaches Weapon
	WeaponMesh->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));
	MulticastHandleDeath(DeathImpulse);
}

/// <summary>
/// 
/// </summary>
/// <param name="DeathImpulse"></param>
void ARPGCharacterBase::MulticastHandleDeath_Implementation(const FVector& DeathImpulse)
{
	UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation(), GetActorRotation());

	// Simulate Physics for weapon
	WeaponMesh->SetSimulatePhysics(true);
	WeaponMesh->SetEnableGravity(true);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	WeaponMesh->AddImpulse(DeathImpulse * 0.1, NAME_None, true);

	// Simulate Physics for Characters
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetEnableGravity(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	GetMesh()->AddImpulse(DeathImpulse,NAME_None, true);

	// Capsule
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Dissolve();

	bDead = true;

	// Manually Deactivating the debuff components
	BurnDebuffComponent->Deactivate();
	StunDebuffComponent->Deactivate();

	OnDeath.Broadcast(this);
}

/// <summary>
/// 
/// </summary>
/// <returns></returns>
UAbilitySystemComponent* ARPGCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

/// <summary>
/// 
/// </summary>
/// <param name="DamageAmount"></param>
/// <param name="DamageEvent"></param>
/// <param name="EventInstigator"></param>
/// <param name="DamageCauser"></param>
/// <returns></returns>
float ARPGCharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float DamageTaken = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	OnDamageDelegate.Broadcast(DamageTaken);
	return DamageTaken;
}

/// <summary>
/// 
/// </summary>
/// <param name="MontageTag"></param>
/// <returns></returns>
FVector ARPGCharacterBase::GetCombatSocketLocation_Implementation(const FGameplayTag& MontageTag)
{
	const FRPGGameplayTags& GameplayTags = FRPGGameplayTags::Get();
	// TODO: Return Correct Socket Based on Montage Tag
	if (MontageTag.MatchesTagExact(GameplayTags.CombatSocket_Weapon) && IsValid(WeaponMesh))
	{
		return WeaponMesh->GetSocketLocation(WeaponTipSocketName);
	}

	if (MontageTag.MatchesTagExact(GameplayTags.CombatSocket_LeftHand))
	{
		return GetMesh()->GetSocketLocation(LeftHandSocketName);
	}

	if (MontageTag.MatchesTagExact(GameplayTags.CombatSocket_RightHand))
	{
		return GetMesh()->GetSocketLocation(RightHandSocketName);
	}

	if (MontageTag.MatchesTagExact(GameplayTags.CombatSocket_Tail))
	{
		return GetMesh()->GetSocketLocation(TailSocketName);
	}

	return FVector();
}

bool ARPGCharacterBase::IsDead_Implementation() const
{
	return bDead;
}

AActor* ARPGCharacterBase::GetAvatar_Implementation()
{
	return this;
}

FTaggedMontage ARPGCharacterBase::GetTaggedMontageByTag_Implementation(const FGameplayTag& MontageTag)
{
	for (FTaggedMontage TaggedMontage : AttackMontages)
	{
		if (TaggedMontage.MontageTag == MontageTag)
		{
			return TaggedMontage;
		}
	}

	return FTaggedMontage();
}

