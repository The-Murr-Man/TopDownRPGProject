// Copyright Kyle Murray


#include "Actor/RPGEffectActor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/RPGAttributeSet.h"

// Sets default values
ARPGEffectActor::ARPGEffectActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	SetRootComponent(Mesh);

	Sphere = CreateDefaultSubobject<USphereComponent>("Sphere");
	Sphere->SetupAttachment(GetRootComponent());
}


// Called when the game starts or when spawned
void ARPGEffectActor::BeginPlay()
{
	Super::BeginPlay();
	
	Sphere->OnComponentBeginOverlap.AddDynamic(this, &ARPGEffectActor::OnOverlap);
	Sphere->OnComponentEndOverlap.AddDynamic(this, &ARPGEffectActor::EndOverlap);
}

void ARPGEffectActor::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//TODO Change this to apply a gameplay effect, for now using const_cast as a hack
	if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(OtherActor))
	{
		const URPGAttributeSet* RPGAttributeSet = Cast<URPGAttributeSet>(ASCInterface->GetAbilitySystemComponent()->GetAttributeSet(URPGAttributeSet::StaticClass()));

		URPGAttributeSet* MutableRPGAttributeSet = const_cast<URPGAttributeSet*>(RPGAttributeSet); // THIS IS BAD DONT DO THIS, ONLY USED FOR DEMONSTRATION

		MutableRPGAttributeSet->SetHealth(RPGAttributeSet->GetHealth() + 25);
		Destroy();
	}
}

void ARPGEffectActor::EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}


