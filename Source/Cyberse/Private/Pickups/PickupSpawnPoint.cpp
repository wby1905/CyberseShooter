// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/PickupSpawnPoint.h"

#include "Pickups/Pickup.h"

APickupSpawnPoint::APickupSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

}

void APickupSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APickupSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
        StartSpawnTimer((AActor*)nullptr);
    }
	
}

void APickupSpawnPoint::SpawnPickup()
{
	if (!HasAuthority() || PickupClasses.Num() <= 0) return;
	int32 RandomIndex = FMath::RandRange(0, PickupClasses.Num() - 1);
	if (PickupClasses[RandomIndex])
	{
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        SpawnedPickup = GetWorld()->SpawnActor<APickup>(PickupClasses[RandomIndex], GetActorLocation(), GetActorRotation(), SpawnParams);
		if (SpawnedPickup)
		{
            SpawnedPickup->OnDestroyed.AddDynamic(this, &APickupSpawnPoint::StartSpawnTimer);
        }
	}
}

void APickupSpawnPoint::StartSpawnTimer(AActor* DestroyedActor)
{
	const float SpawnDelay = FMath::RandRange(SpawnDelayMin, SpawnDelayMax);
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &APickupSpawnPoint::SpawnPickupTimerFinished, SpawnDelay, false);
}

void APickupSpawnPoint::SpawnPickupTimerFinished()
{
	if (HasAuthority())
	{
		SpawnPickup();
	}
}
