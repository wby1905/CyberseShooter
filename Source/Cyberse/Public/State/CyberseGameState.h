// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "CyberseGameState.generated.h"

class ABlasterPlayerState;

UCLASS()
class CYBERSE_API ACyberseGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void UpdateTopScore(ABlasterPlayerState* ScoringPlayer);

	FString GetTopScoringPlayerNames() const;
	FORCEINLINE TArray<ABlasterPlayerState*> GetTopScoringPlayers() const { return TopScoringPlayers; }
private:
	UPROPERTY(Replicated)
		TArray<ABlasterPlayerState*> TopScoringPlayers;

	float TopScore = 0.f;
};
