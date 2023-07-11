// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DamageUI.generated.h"

/**
 * 
 */
UCLASS()
class CYBERSE_API UDamageUI : public UUserWidget
{
	GENERATED_BODY()
	

public:

	void SetDamageText(int32 Damage, FLinearColor Color);
protected:
	virtual void NativeConstruct() override;

private:
	UPROPERTY(meta = (BindWidget))
		TObjectPtr<class UTextBlock> DamageText;

	UPROPERTY(EditDefaultsOnly)
		float DamageShowTime = 1.0f;
	float CurrentDamage = 0.0f;

	FTimerHandle DamageTimerHandle;
	void DamageTimerExpired();
};
