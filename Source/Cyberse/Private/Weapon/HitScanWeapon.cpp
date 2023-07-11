// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundBase.h"

#include "Character/BlasterCharacter.h"
#include "Controller/BlasterPlayerController.h"
#include "Weapon/WeaponTypes.h"
#include "CyberseComponents/LagCompensationComponent.h"

#include "DrawDebugHelpers.h"

AHitScanWeapon::AHitScanWeapon()
{
    FireType = EFireType::EFT_HitScan;
}

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
    Super::Fire(HitTarget);

    if (PlayerCharacter == nullptr) return;

    const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
    if (MuzzleFlashSocket)
    {
        FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
        FVector TraceStart = SocketTransform.GetLocation();        
        FHitResult FireResult;
        WeaponTraceHit(TraceStart, HitTarget, FireResult);
        PlayMuzzleFlash(TraceStart);

        if (FireResult.bBlockingHit)
        {
            ABlasterCharacter* HitCharacter = Cast<ABlasterCharacter>(FireResult.GetActor());
            if (HitCharacter && PlayerController)
            {
                // Server handle damage if not using server side rewind except for server controlled character
                if (HasAuthority() && (!bUseServerSideRewind || !PlayerCharacter->GetLagCompensationComponent() || PlayerCharacter->IsLocallyControlled()))
                {
                    float TotalDamage = Damage;
                    // Check Headshot
                    if (FireResult.BoneName == FName("head"))
                    {
                        TotalDamage *= HeadshotMultiplier;
                    }

                    UGameplayStatics::ApplyDamage(
                        HitCharacter,
                        TotalDamage,
                        PlayerController,
                        this,
                        UDamageType::StaticClass()
                    );
                }
                // Client Request Server to handle damage if using server side rewind
                else if (!HasAuthority() && bUseServerSideRewind && PlayerCharacter->IsLocallyControlled())
                {
                    if (PlayerCharacter->GetLagCompensationComponent())
                    {
                        PlayerCharacter->GetLagCompensationComponent()->ServerScoreRequest(
                            HitCharacter,
                            TraceStart,
                            HitTarget,
                            PlayerController->GetServerTime() - PlayerController->SingleTripTime, // Rewind to the past
                            this
                        );
                    }
                }

                SpawnBodySVFX(FireResult);
                return;
            }

            SpawnSVFX(FireResult);

        }
    }
}

void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& FireResult)
{
    UWorld* World = GetWorld();
    if (World == nullptr) return;
    FVector End = TraceStart + (HitTarget - TraceStart) * 1.25f;

    World->LineTraceSingleByChannel(
        FireResult,
        TraceStart,
        End,
        ECollisionChannel::ECC_Visibility
    );

    SpawnBeam(FireResult, TraceStart, End);
}


void AHitScanWeapon::SpawnBeam(const FHitResult& FireResult, const FVector& TraceStart, const FVector& End)
{
    UWorld* World = GetWorld();
    if (World == nullptr) return;
    // Spawn Beam
    FVector BeamEnd = FireResult.bBlockingHit ? FireResult.ImpactPoint : End;

    //DrawDebugSphere(World, BeamEnd, 10.0f, 8, FColor::Red, false, 4.f);

    if (Beam)
    {
        UParticleSystemComponent* BeamComponent = UGameplayStatics::SpawnEmitterAtLocation(
            World,
            Beam,
            TraceStart,
            FRotator::ZeroRotator
        );
        if (BeamComponent)
        {
            BeamComponent->SetVectorParameter(FName("Target"), BeamEnd);
        }
    }
}

void AHitScanWeapon::PlayMuzzleFlash(const FVector& TraceStart)
{
    UWorld* World = GetWorld();
    if (World == nullptr) return;
    if (MuzzleFlash)
    {
        UGameplayStatics::SpawnEmitterAtLocation(
            World,
            MuzzleFlash,
            TraceStart,
            FRotator::ZeroRotator
        );
    }

    if (FireSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            FireSound,
            GetActorLocation()
        );
    }
}

void AHitScanWeapon::SpawnSVFX(const FHitResult& FireResult)
{
    UWorld* World = GetWorld();
    if (World == nullptr) return;
    if (ImpactParticles)
    {
        UGameplayStatics::SpawnEmitterAtLocation(
            World,
            ImpactParticles,
            FireResult.ImpactPoint,
            FireResult.ImpactNormal.Rotation()
        );
    }

    if (ImpactSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            ImpactSound,
            FireResult.ImpactPoint
        );
    }
}

void AHitScanWeapon::SpawnBodySVFX(const FHitResult& FireResult)
{
    UWorld* World = GetWorld();
    if (World == nullptr) return;
    if (ImpactParticlesOnBody)
    {
        UGameplayStatics::SpawnEmitterAtLocation(
            World,
            ImpactParticlesOnBody,
            FireResult.ImpactPoint,
            FireResult.ImpactNormal.Rotation()
        );
    }

    if (ImpactSoundOnBody)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            ImpactSoundOnBody,
            FireResult.ImpactPoint
        );
    }
}
