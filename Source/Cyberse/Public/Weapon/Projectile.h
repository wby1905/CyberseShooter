// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class UParticleSystem;

UCLASS()
class CYBERSE_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();
	virtual void Tick(float DeltaTime) override;

	/**
	* Server-side rewind
	*/
	bool bUseServerSideRewind = false;
	FVector_NetQuantize100 TraceStart;
	FVector_NetQuantize100 LaunchVelocity;

	FORCEINLINE TObjectPtr<UStaticMeshComponent> GetProjectileMesh() const { return ProjectileMesh; }
	FORCEINLINE void SetDamage(float NewDamage) { Damage = NewDamage; }
	FORCEINLINE float GetDamage() const { return Damage; }
	FORCEINLINE void SetHeadshotMultiplier(float NewHeadshotMultiplier) { HeadshotMultiplier = NewHeadshotMultiplier; }
	FORCEINLINE float GetHeadshotMultiplier() const { return HeadshotMultiplier; }
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	virtual void StartTrailDestroyTimer();
	virtual void DestroyTrail();

	UFUNCTION(NetMulticast, Reliable)
		virtual void MulticastSpawnImpactEffects(FVector_NetQuantize ImpactPoint, AActor* HittedActor);

protected:
	UPROPERTY(VisibleAnywhere)
		TObjectPtr<UStaticMeshComponent> ProjectileMesh; // Only create when the mesh is needed

	UPROPERTY(EditDefaultsOnly)
		TObjectPtr<class UBoxComponent> CollisionBox;

	UPROPERTY(VisibleAnywhere)
		TObjectPtr<class UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(EditDefaultsOnly)
		TObjectPtr<UParticleSystem> Tracer;

	TObjectPtr<class UParticleSystemComponent> TracerComponent;

	UPROPERTY(EditDefaultsOnly)
		TObjectPtr<UParticleSystem> ImpactParticles;

	UPROPERTY(EditDefaultsOnly)
		TObjectPtr<USoundBase> ImpactSound;

	UPROPERTY(EditDefaultsOnly)
		TObjectPtr<UParticleSystem> ImpactParticlesBody;

	UPROPERTY(EditDefaultsOnly)
		TObjectPtr<USoundBase> ImpactSoundBody;

	// set from projectileweapon (manually set for grenade)
	UPROPERTY(EditDefaultsOnly)
		float Damage = 5.f;
	float HeadshotMultiplier = 2.f;


	/**
	* Explosion
	*/
	UPROPERTY(EditDefaultsOnly)
		float MinimumDamage = 10.f;
	UPROPERTY(EditDefaultsOnly)
		float InnerRadius = 100.f;
	UPROPERTY(EditDefaultsOnly)
		float OuterRadius = 300.f;
	UPROPERTY(EditDefaultsOnly)
		float DamageFalloff = 0.5f;

	void Explode();
private:
	UPROPERTY(EditDefaultsOnly)
		float TrailDestroyDelay = 4.5f;
	FTimerHandle TrailDestroyTimer;
};
