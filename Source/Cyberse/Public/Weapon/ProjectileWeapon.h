// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Weapon.h"
#include "ProjectileWeapon.generated.h"

/**
 * 
 */
UCLASS()
class CYBERSE_API AProjectileWeapon : public AWeapon
{
	GENERATED_BODY()

public:
	virtual void Fire(const FVector& HitTarget) override;

private: 

	UPROPERTY(EditDefaultsOnly, Category = "Weapon Properties")
		TSubclassOf<class AProjectile> ProjectileClass;
	
	// not replicated (others are the same as ProjectileClass)
	UPROPERTY(EditDefaultsOnly, Category = "Weapon Properties")
		TSubclassOf<AProjectile> ServerSideRewindProjectileClass;
};
