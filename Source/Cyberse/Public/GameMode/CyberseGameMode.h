// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "CyberseGameMode.generated.h"

namespace MatchState
{
	extern CYBERSE_API const FName CoolDown; // Match duration has ended, displaying cooldown timer and winner
}


UCLASS()
class CYBERSE_API ACyberseGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	ACyberseGameMode();
	virtual void Tick(float DeltaSeconds) override;
	virtual void PlayerEliminated(class ABlasterCharacter* ElimmedCharacter, class ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController);
	virtual void RequestRespawn(class ACharacter* ElimmedCharacter, AController* ElimmedController);

	void PlayerLeftGame(class ABlasterPlayerState* PlayerLeaving);


	UPROPERTY(EditDefaultsOnly)
		float WarmupTime = 10.f;
	UPROPERTY(EditDefaultsOnly)
        float MatchTime = 120.f;
	UPROPERTY(EditDefaultsOnly)
		float CoolDownTime = 10.f;

	float LevelStartingTime = 0.f;

	FORCEINLINE float GetCountdownTime() const { return CountdownTime; }
protected:
	void BeginPlay() override;
	void OnMatchStateSet() override;

private:
	float CountdownTime = 0.f;

	void BroadcastKillFeed(const FString& Killer, const FString& Victim, const FString& KillFeedText);
};
