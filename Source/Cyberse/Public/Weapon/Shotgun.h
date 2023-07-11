// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/HitScanWeapon.h"
#include "Shotgun.generated.h"

/**
 * 
 */
UCLASS()
class CYBERSE_API AShotgun : public AHitScanWeapon
{
	GENERATED_BODY()
public:
	AShotgun();
	// Shotgun has a different fire function that takes in an array of hit targets
	void FireShotgun(const TArray<FVector_NetQuantize100>& HitTargets);

	void ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize100>& TraceEnds);
private:

	UPROPERTY(EditDefaultsOnly)
        uint8 PelletCount = 10;
};
