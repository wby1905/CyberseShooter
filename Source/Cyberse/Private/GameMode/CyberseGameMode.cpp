// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/CyberseGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

#include "Character/BlasterCharacter.h"
#include "Controller/BlasterPlayerController.h"
#include "State/BlasterPlayerState.h"
#include "State/CyberseGameState.h"

namespace MatchState
{
    const FName CoolDown = FName(TEXT("CoolDown"));
}

ACyberseGameMode::ACyberseGameMode()
{
    bDelayedStart = true;

}

void ACyberseGameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (GetMatchState() == MatchState::WaitingToStart)
    {
        CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
        if (CountdownTime <= 0.f)
        {
            StartMatch();
        }
    }
    else if (MatchState == MatchState::InProgress)
    {
        CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
        if (CountdownTime <= 0.f)
        {
            SetMatchState(MatchState::CoolDown);
        }
    }
    else if (MatchState == MatchState::CoolDown)
    {
        CountdownTime = WarmupTime + MatchTime + CoolDownTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
        if (CountdownTime <= 0.f)
        {
            RestartGame();
        }
    }
}

void ACyberseGameMode::PlayerEliminated(ABlasterCharacter* ElimmedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
    ABlasterPlayerState* AttackPlayerState = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;
    ABlasterPlayerState* VictimPlayerState = VictimController ? Cast<ABlasterPlayerState>(VictimController->PlayerState) : nullptr;
    ACyberseGameState* CyberseGameState = GetGameState<ACyberseGameState>();
    
    if (AttackPlayerState && ElimmedCharacter)
    {
        FString KillFeedText = FString(TEXT("Killed"));
        if (AttackPlayerState != VictimPlayerState)
        {
            ABlasterCharacter* Killer = Cast<ABlasterCharacter>(AttackPlayerState->GetPawn());

            TArray<ABlasterPlayerState*> TopScoringPlayers;
            for (ABlasterPlayerState* LeadPlayer : CyberseGameState->GetTopScoringPlayers())
            {
                TopScoringPlayers.Add(LeadPlayer);
            }

            AttackPlayerState->AddToScore(1.f);
            ElimmedCharacter->Elim(AttackPlayerState->GetPlayerName(), false);

            if (CyberseGameState)
            {
                CyberseGameState->UpdateTopScore(AttackPlayerState);
            }

            // new top scoring player
            if (CyberseGameState->GetTopScoringPlayers().Contains(AttackPlayerState) && !TopScoringPlayers.Contains(AttackPlayerState) && Killer)
            {
                Killer->MulticastToggleTheLead(true);
                BroadcastKillFeed(AttackPlayerState->GetPlayerName(), FString(" "), FString("becomes the new lead!"));
            }

            // lost top scoring player
            for (ABlasterPlayerState* Loser : TopScoringPlayers)
            {
                ABlasterCharacter* LostCharacter = Cast<ABlasterCharacter>(Loser->GetPawn());
                if (!CyberseGameState->GetTopScoringPlayers().Contains(Loser) && LostCharacter)
                {
                    LostCharacter->MulticastToggleTheLead(false);
                }
            }

            if (Killer)
            {
                KillFeedText = Killer->GetWeaponName();
            }
        }
        else
        {
            ElimmedCharacter->Elim(FString("Yourself"), false);
        }

        if (VictimPlayerState)
        {
            BroadcastKillFeed(AttackPlayerState->GetPlayerName(), VictimPlayerState->GetPlayerName(), KillFeedText);
        }
    }
    if (VictimPlayerState)
    {
        VictimPlayerState->AddToDeathCount(1);
    }
}

void ACyberseGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
    if (ElimmedController)
    {   ElimmedController->UnPossess();
        RestartPlayerAtPlayerStart(ElimmedController, FindPlayerStart(ElimmedController));
    }
    if (ElimmedCharacter)
    {
        ElimmedCharacter->Destroy();
    }
}

void ACyberseGameMode::PlayerLeftGame(ABlasterPlayerState* PlayerLeaving)
{
    if (PlayerLeaving == nullptr) return;
    ACyberseGameState* CyberseGameState = GetGameState<ACyberseGameState>();
    if (CyberseGameState && CyberseGameState->GetTopScoringPlayers().Contains(PlayerLeaving))
    {
        CyberseGameState->GetTopScoringPlayers().Remove(PlayerLeaving);
    }

    ABlasterCharacter* ElimmedCharacter = Cast<ABlasterCharacter>(PlayerLeaving->GetPawn());
    if (ElimmedCharacter)
    {
        ElimmedCharacter->Elim(FString("Left Game"), true);
    }
}

void ACyberseGameMode::BeginPlay()
{
    Super::BeginPlay();

    LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void ACyberseGameMode::OnMatchStateSet()
{
    Super::OnMatchStateSet();

    FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator();
    for (; Iterator; ++Iterator)
    {
        ABlasterPlayerController* PlayerController = Cast<ABlasterPlayerController>(Iterator->Get());
        if (PlayerController)
        {
            PlayerController->OnMatchStateSet(GetMatchState());
        }
    }

}
void ACyberseGameMode::BroadcastKillFeed(const FString& Killer, const FString& Victim, const FString& KillFeedText)
{
    for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
    {
        ABlasterPlayerController* PlayerController = Cast<ABlasterPlayerController>(Iterator->Get());
        if (PlayerController)
        {
            PlayerController->ClientSetHUDKillFeed(Killer, Victim, KillFeedText);
        }
    }
}