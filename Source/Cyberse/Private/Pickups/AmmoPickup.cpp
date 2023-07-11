// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/AmmoPickup.h"

#include "Character/BlasterCharacter.h"
#include "CyberseComponents/CombatComponent.h"

void AAmmoPickup::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    Super::OnSphereBeginOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

    if (ABlasterCharacter* Character = Cast<ABlasterCharacter>(OtherActor))
    {
        if (UCombatComponent* CombatComponent = Character->GetCombatComponent())
        {
            CombatComponent->PickupAmmo(WeaponType, AmmoAmount);
            Destroy();
        }
    }
}
