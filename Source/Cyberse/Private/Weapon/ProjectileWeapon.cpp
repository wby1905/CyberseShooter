// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"

#include "Weapon/Projectile.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
    Super::Fire(HitTarget);

    APawn* InstigatorPawn = Cast<APawn>(GetOwner());

    const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
    if (MuzzleFlashSocket && InstigatorPawn)
    {
        FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
        // from muzzle to hit location which is from TraceUnderCrosshair
        FVector ToTarget = HitTarget - SocketTransform.GetLocation();
        FRotator ToTargetRotation = ToTarget.Rotation();
        // if not using SSR then use default settings (replicated projectile)
        TSubclassOf<AProjectile> ProjectileClassToSpawn = ProjectileClass;
        bool bProjectileUseServerSideRewind = false;
        if (bUseServerSideRewind)
        {
            // spawn non-replicated projectile
            ProjectileClassToSpawn = ServerSideRewindProjectileClass;
            bProjectileUseServerSideRewind = true;
        }
        if (UWorld* World = GetWorld())
        {
            if (ProjectileClassToSpawn &&
                (bUseServerSideRewind || (InstigatorPawn->HasAuthority() && !bUseServerSideRewind))
                )
            {
                FActorSpawnParameters SpawnParams;
                SpawnParams.Owner = GetOwner();
                SpawnParams.Instigator = InstigatorPawn;

                AProjectile* Projectile = World->SpawnActor<AProjectile>(ProjectileClassToSpawn, SocketTransform.GetLocation(), ToTargetRotation, SpawnParams);
                if (Projectile)
                {
                    Projectile->SetDamage(Damage);
                    Projectile->SetHeadshotMultiplier(HeadshotMultiplier);
                    Projectile->bUseServerSideRewind = bProjectileUseServerSideRewind;
                }
            }
        }
    }
}
