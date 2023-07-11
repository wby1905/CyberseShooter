// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileGrenade.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"
#include "Components/BoxComponent.h"
#include "Sound/SoundBase.h"
#include "GameFramework/ProjectileMovementComponent.h"


AProjectileGrenade::AProjectileGrenade()
{
    ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
    ProjectileMesh->SetupAttachment(RootComponent);
    ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (ProjectileMovement == nullptr)
		ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->SetIsReplicated(true);
	ProjectileMovement->bShouldBounce = true;
}

void AProjectileGrenade::BeginPlay()
{
    AActor::BeginPlay();

	if (Tracer)
	{
		TracerComponent = UGameplayStatics::SpawnEmitterAttached(
			Tracer,
			CollisionBox,
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition);
	}
	if (CollisionBox && HasAuthority())
	{
		CollisionBox->IgnoreActorWhenMoving(GetInstigator(), true);
	}

	StartTrailDestroyTimer();

	ProjectileMovement->OnProjectileBounce.AddDynamic(this, &AProjectileGrenade::OnBounce);
}

void AProjectileGrenade::Destroyed()
{
	MulticastSpawnImpactEffects(GetActorLocation(), nullptr);
	Explode();
	Super::Destroyed();
}

void AProjectileGrenade::OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	if (BounceSound)
	{
        UGameplayStatics::PlaySoundAtLocation(this, BounceSound, GetActorLocation());
    }
}
