// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Projectile.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"

#include "../Cyberse.h"
#include "Character/BlasterCharacter.h"
#include "CyberseComponents/LagCompensationComponent.h"
#include "Controller/BlasterPlayerController.h"
#include "HUD/CyberseHUD.h"
#include "Interfaces/InteractWithCrosshairInterface.h"

AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECR_Block);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->SetIsReplicated(true);
}



void AProjectile::BeginPlay()
{
	Super::BeginPlay();

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
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
		CollisionBox->IgnoreActorWhenMoving(GetOwner(), true);
	}

	TraceStart  = GetActorLocation();
	LaunchVelocity = GetActorForwardVector() * ProjectileMovement->InitialSpeed;
}


void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor)
	{
		if (ABlasterCharacter* Shooter = Cast<ABlasterCharacter>(GetInstigator()))
		{
			ABlasterPlayerController* ShooterController = Cast<ABlasterPlayerController>(Shooter->GetController());
			if (ShooterController)
			{
				if (Shooter->HasAuthority() && (!bUseServerSideRewind || !Shooter->GetLagCompensationComponent() || Shooter->IsLocallyControlled()))
				{
					float TotalDamage = Damage;
					// Check Headshot
					if (Hit.BoneName == FName("head"))
					{
						TotalDamage *= HeadshotMultiplier;
					}
					UGameplayStatics::ApplyDamage(OtherActor, TotalDamage, ShooterController, this, UDamageType::StaticClass());
				}
				else if (!Shooter->HasAuthority() && bUseServerSideRewind && Shooter->IsLocallyControlled())
				{
					ABlasterCharacter* HitCharacter = Cast<ABlasterCharacter>(OtherActor);
					AWeapon* Weapon = Shooter->GetEquippedWeapon();
					if (Shooter->GetLagCompensationComponent())
					{
						Shooter->GetLagCompensationComponent()->ServerProjectileScoreRequest(
							HitCharacter,
							TraceStart,
							LaunchVelocity,
							ShooterController->GetServerTime() - ShooterController->SingleTripTime,
							Weapon);
					}
                }
			}
		}
	}
	if (TracerComponent)
		TracerComponent->DestroyComponent();
	MulticastSpawnImpactEffects(GetActorLocation(), OtherActor);
	Destroy();
}

void AProjectile::StartTrailDestroyTimer()
{
	GetWorldTimerManager().SetTimer(TrailDestroyTimer, this, &AProjectile::DestroyTrail, TrailDestroyDelay, false);
}

void AProjectile::DestroyTrail()
{
	Destroy();
}

void AProjectile::Explode()
{
	if (!HasAuthority()) return;
	ABlasterCharacter* FiringPawn = Cast<ABlasterCharacter>(GetInstigator());
	if (FiringPawn)
	{
		AController* FiringController = FiringPawn->GetController();
		if (FiringController)
		{
			bool hitted = UGameplayStatics::ApplyRadialDamageWithFalloff(
				this,
				Damage,
				MinimumDamage,
				GetActorLocation(),
				InnerRadius,
				OuterRadius,
				DamageFalloff,
				UDamageType::StaticClass(),
				TArray<AActor*>(),
				this,
				FiringController
			);

			if (hitted)
			{
				FiringPawn->ClientSetCrosshairColor(FColor::Red);
			}
		}
	}
}

void AProjectile::MulticastSpawnImpactEffects_Implementation(FVector_NetQuantize ImpactPoint, AActor* HittedActor)
{
	ABlasterCharacter* Character = Cast<ABlasterCharacter>(HittedActor);
	if (Character && ImpactParticlesBody && ImpactSoundBody)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticlesBody, ImpactPoint, FRotator::ZeroRotator, true);
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactSoundBody, ImpactPoint);
		return;
	}

	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}

	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactSound, GetActorLocation());
	}
}

