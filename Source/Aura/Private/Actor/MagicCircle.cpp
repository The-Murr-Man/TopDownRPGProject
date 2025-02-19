// Copyright Kyle Murray


#include "Actor/MagicCircle.h"
#include "Components/SphereComponent.h"
#include "Interaction/EnemyInterface.h"

// Sets default values
AMagicCircle::AMagicCircle()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	TargetCircle = CreateDefaultSubobject<USphereComponent>("Magic Circle");
	SetRootComponent(TargetCircle);
	TargetCircle->SetCollisionResponseToChannels(ECR_Ignore);
	TargetCircle->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	MagicCircleDecal = CreateDefaultSubobject<UDecalComponent>("Magic Circle Decal");
	MagicCircleDecal->SetupAttachment(GetRootComponent());

}

// Called when the game starts or when spawned
void AMagicCircle::BeginPlay()
{
	Super::BeginPlay();

	TargetCircle->OnComponentBeginOverlap.AddDynamic(this, &AMagicCircle::OnTargetingBeginOverlap);
	TargetCircle->OnComponentEndOverlap.AddDynamic(this, &AMagicCircle::OnTargetingEndOverlap);
}

// Called every frame
void AMagicCircle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMagicCircle::OnTargetingBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IEnemyInterface* Enemy = Cast<IEnemyInterface>(OtherActor))
	{
		Enemy->HighlightActor();
	}
}

void AMagicCircle::OnTargetingEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (IEnemyInterface* Enemy = Cast<IEnemyInterface>(OtherActor))
	{
		Enemy->UnHighlightActor();
	}
}

void AMagicCircle::SetMagicCircleRadius(float InRadius)
{
	TargetCircle->SetSphereRadius(InRadius);
	MagicCircleDecal->DecalSize = FVector(InRadius);
}

