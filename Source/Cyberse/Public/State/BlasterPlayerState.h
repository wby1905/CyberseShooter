// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BlasterPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class CYBERSE_API ABlasterPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	virtual void OnRep_Score() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void AddToScore(float ScoreAmount);
	void AddToDeathCount(int32 Amount);
	FORCEINLINE TObjectPtr<class ABlasterCharacter> GetCharacter() const { return Character; }
private:
	UPROPERTY()
		TObjectPtr<ABlasterCharacter> Character;
	UPROPERTY()
		TObjectPtr<class ABlasterPlayerController> Controller;

	UPROPERTY(ReplicatedUsing = OnRep_DeathCount)
		int32 DeathCount;

	UFUNCTION()
		void OnRep_DeathCount();

};
