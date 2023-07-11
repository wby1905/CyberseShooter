// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverheadWidget.generated.h"

/**
 * 
 */
UCLASS()
class CYBERSE_API UOverheadWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetDisplayText(const FString& Text);
	void ShowPlayerNetRole(APawn* PlayerPawn);

protected:
	virtual void NativeDestruct() override;

private:
	UPROPERTY(meta = (BindWidget))
		TObjectPtr<class UTextBlock> DisplayText;
	
};
