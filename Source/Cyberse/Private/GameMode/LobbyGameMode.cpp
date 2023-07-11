// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    int32 NumOfPlayers = GameState.Get()->PlayerArray.Num();
    if (NumOfPlayers >= MinPlayers)
    {
        UWorld* World = GetWorld();
        if (World)
        {
            bUseSeamlessTravel = true;
            World->ServerTravel(FString("/Game/Maps/CyberseMap?listen"));
        }
    }
}
