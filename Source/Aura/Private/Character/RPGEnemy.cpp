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
	
	/// Setup ASC
	AbilitySystemComponent = CreateDefaultSubobject<URPGAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Full);

	// Turn off Controller rotations
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bUseControllerDesiredRotation = true;

	// Create Attribute Set
	AttributeSet = CreateDefaultSubobject<URPGAttributeSet>("AttributeSet");

	// Create Health Bar Widget
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

	// Initializes Blackboard on AI Controller
	RPGAIController->GetBlackboardComponent()->InitializeBlackboard(*BehaviorTree->BlackboardAsset);

	// Runs Behavior Tree
	RPGAIController->RunBehaviorTree(BehaviorTree);

	// Set Blackboard bools
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
		// Give Startup abilities
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
/// Calls nessasary fucntions to Initailize all actor ability info
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
/// Set default attributes based on class and level
/// </summary>
void ARPGEnemy::InitializeDefaultAttributes()
{
	URPGAbilitySystemLibrary::InitializeDefaultAttributes(this, CharacterClass, Level, AbilitySystemComponent);
}

/// <summary>
/// Updates stun tag
/// </summary>
/// <param name="CallbackTag"></param>
/// <param name="NewCount"></param>
void ARPGEnemy::StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	Super::StunTagChanged(CallbackTag, NewCount);

	if (RPGAIController && RPGAIController->GetBlackboardComponent())
	{
		RPGAIController->GetBlackboardComponent()->SetValueAsBool(FName("Stunned"), bIsStunned);
	}
}

/// <summary>
/// Updates Hit React Tag
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
/// Highlights Actor
/// </summary>
void ARPGEnemy::HighlightActor_Implementation()
{
	GetMesh()->SetRenderCustomDepth(true);
	WeaponMesh->SetRenderCustomDepth(true);
}

/// <summary>
/// UnHighlights Actor
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

/// <summary>
/// Sets combat target
/// </summary>
/// <param name="InCombatTarget"></param>
void ARPGEnemy::SetCombatTarget_Implementation(AActor* InCombatTarget)
{
	CombatTarget = InCombatTarget;
}

/// <summary>
/// Returns Combat Target
/// </summary>
/// <returns></returns>
AActor* ARPGEnemy::GetCombatTarget_Implementation() const
{
	return CombatTarget;
}

/// <summary>
/// Returns Players Level
/// </summary>
/// <returns></returns>
int32 ARPGEnemy::GetPlayerLevel_Implementation()
{
	return Level;
}

/// <summary>
/// Handles Death functionality
/// </summary>
/// <param name="DeathImpulse"></param>
void ARPGEnemy::Die(const FVector& DeathImpulse)
{
	SetLifeSpan(LifeSpan);

	// Update blackboard bool for death
	if (RPGAIController) RPGAIController->GetBlackboardComponent()->SetValueAsBool(FName("Dead"),true);

	// Spawn loot
	SpawnLoot();
	Super::Die(DeathImpulse);
}


