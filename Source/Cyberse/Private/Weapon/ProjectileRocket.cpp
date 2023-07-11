// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/BoxComponent.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"

#include "Weapon/RocketMovementComponent.h"

AProjectileRocket::AProjectileRocket()
{
    ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
    ProjectileMesh->SetupAttachment(RootComponent);
    ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    FlyingSoundComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("FlyingSoundComponent"));
    FlyingSoundComponent->SetupAttachment(RootComponent);
    FlyingSoundComponent->bAutoActivate = true;
    FlyingSoundComponent->bAutoDestroy = false;
}

void AProjectileRocket::BeginPlay()
{
    Super::BeginPlay();
    if (FlyingSound)
    {
        FlyingSoundComponent->SetSound(FlyingSound);
        FlyingSoundComponent->Play();
    }
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    Explode();
    StartTrailDestroyTimer();
    MulticastSpawnImpactEffects(GetActorLocation(), OtherActor);
    if (ProjectileMesh)
        ProjectileMesh->SetVisibility(false);
    if (CollisionBox)
        CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (TracerComponent)
    {
        TracerComponent->DeactivateSystem();
    }
    if (FlyingSoundComponent && FlyingSoundComponent->IsPlaying())
    {
        FlyingSoundComponent->Stop();
    }
}
