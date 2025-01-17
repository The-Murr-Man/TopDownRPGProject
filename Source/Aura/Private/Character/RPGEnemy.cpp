// Copyright Kyle Murray


#include "Character/RPGEnemy.h"
#include "Aura/Aura.h"
#include "AbilitySystem/RPGAbilitySystemComponent.h"
#include "AbilitySystem/RPGAttributeSet.h"


ARPGEnemy::ARPGEnemy()
{
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	AbilitySystemComponent = CreateDefaultSubobject<URPGAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	AttributeSet = CreateDefaultSubobject<URPGAttributeSet>("AttributeSet");
}

void ARPGEnemy::BeginPlay()
{
	Super::BeginPlay();

	InitAbilityActorInfo();
}

void ARPGEnemy::InitAbilityActorInfo()
{
	if (!AbilitySystemComponent) return;

	// Setting both owning and avatar actors as this actor
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	Cast<URPGAbilitySystemComponent>(AbilitySystemComponent)->AbilityActorInfoSet();
}

void ARPGEnemy::HighlightActor()
{
	GetMesh()->SetRenderCustomDepth(true);
	GetMesh()->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);

	WeaponMesh->SetRenderCustomDepth(true);
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
}

void ARPGEnemy::UnHighlightActor()
{
	GetMesh()->SetRenderCustomDepth(false);
	WeaponMesh->SetRenderCustomDepth(false);
}

int32 ARPGEnemy::GetPlayerLevel()
{
	return Level;
}
