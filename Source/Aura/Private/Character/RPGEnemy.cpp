// Copyright Kyle Murray


#include "Character/RPGEnemy.h"
#include "Aura/Aura.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "AbilitySystem/RPGAttributeSet.h"
#include "Components/WidgetComponent.h"
#include "UI/Widget/RPGUserWidget.h"
#include "AbilitySystem/RPGAbilitySystemLibrary.h"
#include "RPGGameplayTags.h"
#include "AI/RPGAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "GameFramework/CharacterMovementComponent.h"

ARPGEnemy::ARPGEnemy()
{
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	
	AbilitySystemComponent = CreateDefaultSubobject<URPGAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Full);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bUseControllerDesiredRotation = true;

	AttributeSet = CreateDefaultSubobject<URPGAttributeSet>("AttributeSet");

	HealthBar = CreateDefaultSubobject<UWidgetComponent>("HealthBar");
	HealthBar->SetupAttachment(GetRootComponent());

	GetMesh()->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
	GetMesh()->MarkRenderStateDirty();

	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
	WeaponMesh->MarkRenderStateDirty();

	BaseMaxWalkSpeed = 250;
}

void ARPGEnemy::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	//
	if (!HasAuthority()) return;
	RPGAIController = Cast<ARPGAIController>(NewController);

	RPGAIController->GetBlackboardComponent()->InitializeBlackboard(*BehaviorTree->BlackboardAsset);
	RPGAIController->RunBehaviorTree(BehaviorTree);
	RPGAIController->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), false);

	RPGAIController->GetBlackboardComponent()->SetValueAsBool(FName("RangedAttacker"), CharacterClass != ECharacterClass::Warrior);
	
}

void ARPGEnemy::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->MaxWalkSpeed =  BaseMaxWalkSpeed;
	InitAbilityActorInfo();
	
	// Check to see if player is server for startup abilities
	if (HasAuthority())
	{
		URPGAbilitySystemLibrary::GiveStartupAbilities(this, AbilitySystemComponent, CharacterClass);
	}

	if(URPGUserWidget* RPGUserWidget = Cast<URPGUserWidget>(HealthBar->GetUserWidgetObject()))
	{
		RPGUserWidget->SetWidgetController(this);
	}

	// Binding callbacks
	if (URPGAttributeSet* RPGAS = CastChecked<URPGAttributeSet>(AttributeSet))
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(RPGAS->GetHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnHealthChanged.Broadcast(Data.NewValue);
			}
		);

		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(RPGAS->GetMaxHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnMaxHealthChanged.Broadcast(Data.NewValue);
			}
		);
		
		// Adding a listener for when to apply HitReact Event
		AbilitySystemComponent->RegisterGameplayTagEvent(FRPGGameplayTags::Get().Effects_HitReact, EGameplayTagEventType::NewOrRemoved).AddUObject(this,&ARPGEnemy::HitReactTagChanged);

		OnHealthChanged.Broadcast(RPGAS->GetHealth());
		OnMaxHealthChanged.Broadcast(RPGAS->GetMaxHealth());
	}
}

/// <summary>
/// 
/// </summary>
void ARPGEnemy::InitAbilityActorInfo()
{
	if (!AbilitySystemComponent) return;

	// Setting both owning and avatar actors as this actor
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	Cast<URPGAbilitySystemComponent>(AbilitySystemComponent)->AbilityActorInfoSet();

	AbilitySystemComponent->RegisterGameplayTagEvent(FRPGGameplayTags::Get().Debuff_Stun, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ARPGEnemy::StunTagChanged);

	if (HasAuthority())
	{
		InitializeDefaultAttributes();
	}

	OnASCRegistered.Broadcast(AbilitySystemComponent);
}

/// <summary>
/// 
/// </summary>
void ARPGEnemy::InitializeDefaultAttributes()
{
	URPGAbilitySystemLibrary::InitializeDefaultAttributes(this, CharacterClass, Level, AbilitySystemComponent);
}

void ARPGEnemy::StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	Super::StunTagChanged(CallbackTag, NewCount);

	if (RPGAIController && RPGAIController->GetBlackboardComponent())
	{
		RPGAIController->GetBlackboardComponent()->SetValueAsBool(FName("Stunned"), bIsStunned);
	}
}

/// <summary>
/// 
/// </summary>
/// <param name="CallbackTag"></param>
/// <param name="NewCount"></param>
void ARPGEnemy::HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	bHitReacting = NewCount > 0;
	GetCharacterMovement()->MaxWalkSpeed = bHitReacting ? 0 : BaseMaxWalkSpeed;

	if (RPGAIController && RPGAIController->GetBlackboardComponent())
	{
		RPGAIController->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), bHitReacting);
	}
}

/// <summary>
/// 
/// </summary>
void ARPGEnemy::HighlightActor_Implementation()
{
	GetMesh()->SetRenderCustomDepth(true);
	WeaponMesh->SetRenderCustomDepth(true);
}

/// <summary>
/// 
/// </summary>
void ARPGEnemy::UnHighlightActor_Implementation()
{
	GetMesh()->SetRenderCustomDepth(false);
	WeaponMesh->SetRenderCustomDepth(false);
}

void ARPGEnemy::SetMoveToLocation_Implementation(FVector& OutLocation)
{
	// Do not change out Destination
}

void ARPGEnemy::SetCombatTarget_Implementation(AActor* InCombatTarget)
{
	CombatTarget = InCombatTarget;
}

AActor* ARPGEnemy::GetCombatTarget_Implementation() const
{
	return CombatTarget;
}

int32 ARPGEnemy::GetPlayerLevel_Implementation()
{
	return Level;
}

void ARPGEnemy::Die(const FVector& DeathImpulse)
{
	SetLifeSpan(LifeSpan);

	if (RPGAIController) RPGAIController->GetBlackboardComponent()->SetValueAsBool(FName("Dead"),true);

	SpawnLoot();
	Super::Die(DeathImpulse);
}


