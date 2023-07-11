// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/HealthPickup.h"

#include "Character/BlasterCharacter.h"
#include "CyberseComponents/BuffComponent.h"

void AHealthPickup::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    Super::OnSphereBeginOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

    if (ABlasterCharacter* Character = Cast<ABlasterCharacter>(OtherActor))
    {
        if (UBuffComponent* BuffComponent = Character->GetBuffComponent())
        {
            BuffComponent->Heal(HealthAmount, HealingTime);

            Destroy();
        }
    }
}
