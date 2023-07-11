// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ReturnToMainMenu.generated.h"

/**
 * 
 */
UCLASS()
class CYBERSE_API UReturnToMainMenu : public UUserWidget
{
	GENERATED_BODY()

public:

	void MenuSetup();
	void MenuTeardown();
protected:
	virtual bool Initialize() override;

private:
    UPROPERTY(meta = (BindWidget))
		class UButton* ReturnButton;

	TObjectPtr<class UMultiplayerSessionsSubsystem> MultiplayerSessionsSubsystem;

	TObjectPtr<class APlayerController> PlayerController;


	UFUNCTION()
		void ReturnButtonClicked();

	UFUNCTION()
		void OnDestroySessionComplete(bool bWasSuccessful);

	UFUNCTION()
		void OnPlayerLeftGame();


};
