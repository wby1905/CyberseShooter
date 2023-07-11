// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/OverheadWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"

void UOverheadWidget::SetDisplayText(const FString& Text)
{
    if (DisplayText)
    {
        DisplayText->SetText(FText::FromString(Text));
    }
}

void UOverheadWidget::ShowPlayerNetRole(APawn* PlayerPawn)
{
    if (PlayerPawn)
    {
        FString PlayerName{ "Unknown" };
        APlayerState* PlayerState = PlayerPawn->GetPlayerState();
        if (PlayerState)
        {
            PlayerName = PlayerState->GetPlayerName();
        }
        //SetDisplayText(FString::Printf(TEXT("Name: %s"),*PlayerName));
    }
}


void UOverheadWidget::NativeDestruct()
{
    RemoveFromParent();
    Super::NativeDestruct();
}
