// Fill out your copyright notice in the Description page of Project Settings.


#include "State/BlasterPlayerState.h"
#include "Net/UnrealNetwork.h"

#include "Character/BlasterCharacter.h"
#include "Controller/BlasterPlayerController.h"

void ABlasterPlayerState::OnRep_Score()
{
    Super::OnRep_Score();
    
    Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
    if (Character)
    {
        Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->GetController()) : Controller;
        if (Controller)
        {
            Controller->SetHUDScore(GetScore());
        }
    }
}

void ABlasterPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME_CONDITION(ABlasterPlayerState, DeathCount, COND_OwnerOnly);
}

void ABlasterPlayerState::AddToScore(float ScoreAmount)
{
    // called on server
    SetScore(GetScore() + ScoreAmount);
    OnRep_Score();
}

void ABlasterPlayerState::AddToDeathCount(int32 Amount)
{
    // called on server
    DeathCount += Amount;
    OnRep_DeathCount();
}

void ABlasterPlayerState::OnRep_DeathCount()
{
    Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
    if (Character)
    {
        Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->GetController()) : Controller;
        if (Controller)
        {
            Controller->SetHUDDeathCount(DeathCount);
        }
    }
}
