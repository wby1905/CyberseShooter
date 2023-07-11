// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Weapon.h"
#include "HitScanWeapon.generated.h"

/**
 * 
 */
UCLASS()
class CYBERSE_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()

public:
	AHitScanWeapon();

	virtual void Fire(const FVector& HitTarget) override;
protected:

	UPROPERTY(EditDefaultsOnly)
		TObjectPtr<class UParticleSystem> ImpactParticles;

	UPROPERTY(EditDefaultsOnly)
		TObjectPtr<UParticleSystem> ImpactParticlesOnBody;

	UPROPERTY(EditDefaultsOnly)
		TObjectPtr<UParticleSystem> Beam;

	UPROPERTY(EditDefaultsOnly)
		TObjectPtr<UParticleSystem> MuzzleFlash;

	UPROPERTY(EditDefaultsOnly)
		TObjectPtr<class USoundBase> FireSound;

	UPROPERTY(EditDefaultsOnly)
		TObjectPtr<USoundBase> ImpactSound;

	UPROPERTY(EditDefaultsOnly)
		TObjectPtr<USoundBase> ImpactSoundOnBody;

	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& FireResult);

	void SpawnBeam(const FHitResult& FireResult, const FVector& TraceStart, const FVector& End);

	void PlayMuzzleFlash(const FVector& TraceStart);

	void SpawnSVFX(const FHitResult& FireResult);

	void SpawnBodySVFX(const FHitResult& FireResult);



};
