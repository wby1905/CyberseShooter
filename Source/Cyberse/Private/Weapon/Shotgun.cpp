// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundBase.h"
#include "Kismet/KismetMathLibrary.h"

#include "Character/BlasterCharacter.h"
#include "Controller/BlasterPlayerController.h"
#include "CyberseComponents/LagCompensationComponent.h"

AShotgun::AShotgun()
{
    FireType = EFireType::EFT_Shotgun;
}

void AShotgun::FireShotgun(const TArray<FVector_NetQuantize100>& HitTargets)
{
    AWeapon::Fire(FVector::ZeroVector);

    if (PlayerCharacter == nullptr) return;
    const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
    if (MuzzleFlashSocket)
    {
        const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
        const FVector TraceStart = SocketTransform.GetLocation();
        PlayMuzzleFlash(TraceStart);

        // Maps hit character to Total Damage(multiplied with headshot multiplier)
        TMap<ABlasterCharacter*, float> HitCharacterMap;
        for (FVector_NetQuantize HitTarget : HitTargets)
        {
            FHitResult FireResult;
            WeaponTraceHit(TraceStart, HitTarget, FireResult);
            if (FireResult.bBlockingHit)
            {
                if (ABlasterCharacter* HitCharacter = Cast<ABlasterCharacter>(FireResult.GetActor()))
                {
                    float TotalDamage = Damage;
                    if (FireResult.BoneName == FName("head"))
                    {
                        TotalDamage *= HeadshotMultiplier;
                    }
                    if (HitCharacterMap.Contains(HitCharacter))
                    {
                        HitCharacterMap[HitCharacter]+= TotalDamage;
                    }
                    else
                    {
                        HitCharacterMap.Emplace(HitCharacter, TotalDamage);
                    }
                    SpawnBodySVFX(FireResult);
                }
                else
                {
                    SpawnSVFX(FireResult);
                }
            }
        }
        if (PlayerController)
        {
            // Server handle damage if not using server side rewind except for server controlled character
            if (HasAuthority() && (!bUseServerSideRewind || !PlayerCharacter->GetLagCompensationComponent() || PlayerCharacter->IsLocallyControlled()))
            {
                for (auto& HitPair : HitCharacterMap)
                {
                    if (HitPair.Key == nullptr) continue;
                    UGameplayStatics::ApplyDamage(
                        HitPair.Key,
                        HitPair.Value, // damage is multiplied by the number of pellets that hit the character
                        PlayerController,
                        this,
                        UDamageType::StaticClass()
                    );
                }
            }
            // Client Request Server to handle damage if using server side rewind
            else if (!HasAuthority() && bUseServerSideRewind && PlayerCharacter->IsLocallyControlled())
            {
                TArray<ABlasterCharacter*> HitCharacters;
                for (auto& HitPair : HitCharacterMap)
                {
                    if (HitPair.Key == nullptr) continue;
                    HitCharacters.Add(HitPair.Key);
                }
                if (PlayerCharacter->GetLagCompensationComponent())
                {
                    PlayerCharacter->GetLagCompensationComponent()->ServerShotgunScoreRequest(
                        HitCharacters,
                        TraceStart,
                        HitTargets,
                        PlayerController->GetServerTime() - PlayerController->SingleTripTime, // Rewind to the past
                        this
                    );
                }
            }
        }

    }
}

void AShotgun::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize100>& TraceEnds)
{
    const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
    if (MuzzleFlashSocket == nullptr) return;

    const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
    const FVector TraceStart = SocketTransform.GetLocation();
    const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
    const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;

    // optimized for shotgun (instead of using TraceEndWithScatter)
    for (uint8 i = 0; i < PelletCount; i++)
    {
        const FVector RandomVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
        const FVector EndLoc = SphereCenter + RandomVec;
        const FVector ToEndDirection = (EndLoc - TraceStart).GetSafeNormal();
        TraceEnds.Add(TraceStart + ToEndDirection * TRACE_LENGTH);
    };
}
