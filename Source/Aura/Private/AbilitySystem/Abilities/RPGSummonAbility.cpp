// Copyright Kyle Murray


#include "AbilitySystem/Abilities/RPGSummonAbility.h"

TArray<FVector> URPGSummonAbility::GetSpawnLocations()
{
    const FVector Forward = GetAvatarActorFromActorInfo()->GetActorForwardVector();
    const FVector Location = GetAvatarActorFromActorInfo()->GetActorLocation();

    const float DeltaSpread = SpawnSpread / NumMinions;

    const FVector LeftOfSpread = Forward.RotateAngleAxis(-SpawnSpread * 0.5, FVector::UpVector);
    TArray<FVector> SpawnLocations;

    for (int32 i = 0; i < NumMinions; i++)
    {
        const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread * i,FVector::UpVector);
        FVector ChosenSpawnLocation = Location + Direction * FMath::RandRange(MinSpawnDistance, MaxSpawnDistance);

        FHitResult Hit;

        // Make sure to spawn on the ground
        GetWorld()->LineTraceSingleByChannel(Hit, ChosenSpawnLocation + FVector(0, 0, 400), ChosenSpawnLocation - FVector(0, 0, 400), ECC_Visibility);

        if (Hit.bBlockingHit)
        {
            ChosenSpawnLocation = Hit.ImpactPoint;
        }

        SpawnLocations.Add(ChosenSpawnLocation);
    }

    return SpawnLocations;
}

TSubclassOf<APawn> URPGSummonAbility::GetRandomMinionClass()
{
    const int32 Selection = FMath::RandRange(0, MinionClasses.Num() - 1);

    return MinionClasses[Selection];
}
