// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CYBERSE_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBuffComponent();
	friend class ABlasterCharacter;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void Heal(float HealAmount, float HealTime);
	void Shield(float ShieldAmount, float ShieldTime);

protected:
	virtual void BeginPlay() override;

	void HealTick(float DeltaTime);
	void ShieldTick(float DeltaTime);
private:
	UPROPERTY()
		TObjectPtr<ABlasterCharacter> OwnerCharacter;

	bool bHealing = false;
	float HealingRate = 0.f;
	float AmountToHeal = 0.f;

	bool bShielding = false;
	float ShieldRate = 0.f;
	float AmountToShield = 0.f;
};
