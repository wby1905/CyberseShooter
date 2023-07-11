// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupSpawnPoint.generated.h"

UCLASS()
class CYBERSE_API APickupSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	APickupSpawnPoint();
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(EditAnywhere)
		TArray<TSubclassOf<class APickup>> PickupClasses;

	TObjectPtr<APickup> SpawnedPickup;

	virtual void BeginPlay() override;
	void SpawnPickup();


private:
	UPROPERTY(EditAnywhere)
        float SpawnDelayMin = 3.0f;
	UPROPERTY(EditAnywhere)
		float SpawnDelayMax = 10.0f;

    FTimerHandle SpawnTimerHandle;

	UFUNCTION()
		void StartSpawnTimer(AActor* DestroyedActor);
	void SpawnPickupTimerFinished();
};
