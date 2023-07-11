// Fill out your copyright notice in the Description page of Project Settings.


#include "State/CyberseGameState.h"
#include "Net/UnrealNetwork.h"

#include "State/BlasterPlayerState.h"

void ACyberseGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ACyberseGameState, TopScoringPlayers);
}

void ACyberseGameState::UpdateTopScore(ABlasterPlayerState* ScoringPlayer)
{
    if (TopScoringPlayers.Num() == 0)
    {
        TopScoringPlayers.Add(ScoringPlayer);
        TopScore = ScoringPlayer->GetScore();
    }
    else if (ScoringPlayer->GetScore() > TopScore)
    {
        TopScoringPlayers.Empty();
        TopScoringPlayers.Add(ScoringPlayer);
        TopScore = ScoringPlayer->GetScore();
    }
    else if (ScoringPlayer->GetScore() == TopScore)
    {
        TopScoringPlayers.AddUnique(ScoringPlayer);
    }
}

FString ACyberseGameState::GetTopScoringPlayerNames() const
{
    if (TopScoringPlayers.Num() == 0)
    {
        return FString("No Winner!");
    }
    else if (TopScoringPlayers.Num() == 1)
    {
        return FString::Printf(TEXT("%s Wins!"), *TopScoringPlayers[0]->GetPlayerName());
    }
    else
    {
        FString Names = TEXT("Winners: \n");
        for (int i = 0; i < TopScoringPlayers.Num(); i++)
        {
            Names += TopScoringPlayers[i]->GetPlayerName();
            if (i < TopScoringPlayers.Num() - 1)
            {
                Names += TEXT("\n");
            }
        }
        return Names;
    }
}
