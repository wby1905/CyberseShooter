// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/ReturnToMainMenu.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameModeBase.h"
#include "Components/Button.h"

#include "Character/BlasterCharacter.h"

#include "MultiplayerSessionsSubsystem.h"

void UReturnToMainMenu::MenuSetup()
{
    AddToViewport();
    SetVisibility(ESlateVisibility::Visible);
    bIsFocusable = true;
    if (GetWorld())
    {
        PlayerController = PlayerController == nullptr ? GetWorld()->GetFirstPlayerController() : PlayerController;
        if (PlayerController)
        {
            FInputModeGameAndUI InputModeData;
            InputModeData.SetWidgetToFocus(TakeWidget());
            InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            PlayerController->SetInputMode(InputModeData);
            PlayerController->SetShowMouseCursor(true);
        }
    }

    UGameInstance* GameInstance = GetGameInstance();
    if (GameInstance)
    {
        MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
        if (MultiplayerSessionsSubsystem)
        {
            MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &UReturnToMainMenu::OnDestroySessionComplete);
        }
    }

}

void UReturnToMainMenu::MenuTeardown()
{
    RemoveFromParent();
    SetVisibility(ESlateVisibility::Hidden);
    bIsFocusable = false;
    if (GetWorld())
    {
        PlayerController = PlayerController == nullptr ? GetWorld()->GetFirstPlayerController() : PlayerController;

        if (PlayerController)
        {
            FInputModeGameOnly InputModeData;
            PlayerController->SetInputMode(InputModeData);
            PlayerController->SetShowMouseCursor(false);
        }
    }
    if (MultiplayerSessionsSubsystem)
    {
        MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.RemoveDynamic(this, &UReturnToMainMenu::OnDestroySessionComplete);
    }
}

bool UReturnToMainMenu::Initialize()
{
    if (!Super::Initialize()) return false;
    if (ReturnButton)
    {
        ReturnButton->OnClicked.AddDynamic(this, &UReturnToMainMenu::ReturnButtonClicked);
    }


    return true;
}

void UReturnToMainMenu::ReturnButtonClicked()
{   
    if (ReturnButton) ReturnButton->SetIsEnabled(false);
    if (GetWorld())
    {
        PlayerController = PlayerController == nullptr ? GetWorld()->GetFirstPlayerController() : PlayerController;
        if (PlayerController)
        {
            ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(PlayerController->GetCharacter());
            if (BlasterCharacter)
            {
                BlasterCharacter->ServerLeaveGame();
                BlasterCharacter->OnLeftGame.AddDynamic(this, &UReturnToMainMenu::OnPlayerLeftGame);
            }
            else
            {
                if (ReturnButton) ReturnButton->SetIsEnabled(true);
            }
        }
    }
}

void UReturnToMainMenu::OnDestroySessionComplete(bool bWasSuccessful)
{
    if (!bWasSuccessful && ReturnButton)
    {
        ReturnButton->SetIsEnabled(true);
        return;
    }
    UWorld* World = GetWorld();
    if (World)
    {
        AGameModeBase* GameMode = World->GetAuthGameMode<AGameModeBase>();
        if (GameMode) // server
        {
            GameMode->ReturnToMainMenuHost();
        }
        else // client
        {
            PlayerController = PlayerController == nullptr ? GetWorld()->GetFirstPlayerController() : PlayerController;
            if (PlayerController)
            {
                PlayerController->ClientReturnToMainMenuWithTextReason(FText::FromString("Session Destroyed"));
            }

        }
    }
}

void UReturnToMainMenu::OnPlayerLeftGame()
{
    if (MultiplayerSessionsSubsystem)
    {
        MultiplayerSessionsSubsystem->DestroySession();
    }
}
