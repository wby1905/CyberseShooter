// Fill out your copyright notice in the Description page of Project Settings.


#include "CyberseComponents/BuffComponent.h"

#include "Character/BlasterCharacter.h"

UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	HealTick(DeltaTime);
	ShieldTick(DeltaTime);
}

void UBuffComponent::Heal(float HealAmount, float HealTime)
{
	bHealing = true;
	// if multiple heals are applied, the healing rate should be the same.
	HealingRate = HealAmount / HealTime;
	AmountToHeal += HealAmount;
}

void UBuffComponent::Shield(float ShieldAmount, float ShieldTime)
{
	bShielding = true;
	// if multiple shields are applied, add them together and speed up the rate
    AmountToShield += ShieldAmount;
    ShieldRate = AmountToShield / ShieldTime;
}


void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UBuffComponent::HealTick(float DeltaTime)
{
	if (!bHealing || OwnerCharacter == nullptr || OwnerCharacter->IsElimmed()) return;

	const float HealThisFrame = HealingRate * DeltaTime;
	OwnerCharacter->SetHealth(FMath::Clamp(OwnerCharacter->GetHealth() + HealThisFrame, 0.f, OwnerCharacter->GetMaxHealth()));
	AmountToHeal -= HealThisFrame;

	OwnerCharacter->UpdateHUDHealth();

	if (AmountToHeal <= 0.f || OwnerCharacter->GetHealth() >= OwnerCharacter->GetMaxHealth())
	{
        bHealing = false;
        HealingRate = 0.f;
        AmountToHeal = 0.f;
    }
}

void UBuffComponent::ShieldTick(float DeltaTime)
{
	if (!bShielding || OwnerCharacter == nullptr || OwnerCharacter->IsElimmed()) return;

    const float ShieldThisFrame = ShieldRate * DeltaTime;
    OwnerCharacter->SetShield(FMath::Clamp(OwnerCharacter->GetShield() + ShieldThisFrame, 0.f, OwnerCharacter->GetMaxShield()));
    AmountToShield -= ShieldThisFrame;

    OwnerCharacter->UpdateHUDShield();

	if (AmountToShield <= 0.f || OwnerCharacter->GetShield() >= OwnerCharacter->GetMaxShield())
	{
        bShielding = false;
        ShieldRate = 0.f;
        AmountToShield = 0.f;
    }
}
