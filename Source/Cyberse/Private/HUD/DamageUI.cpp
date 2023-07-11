// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/DamageUI.h"
#include "Components/TextBlock.h"

void UDamageUI::SetDamageText(int32 Damage, FLinearColor Color)
{
    if (GetWorld() == nullptr) return;
    GetWorld()->GetTimerManager().ClearTimer(DamageTimerHandle);
    CurrentDamage += Damage;
    if (DamageText)
    {
        DamageText->SetText(FText::FromString(FString::FromInt(CurrentDamage)));
        DamageText->SetColorAndOpacity(Color);
        SetVisibility(ESlateVisibility::Visible);
    }
    GetWorld()->GetTimerManager().SetTimer(DamageTimerHandle, this, &UDamageUI::DamageTimerExpired, DamageShowTime, false);
}

void UDamageUI::NativeConstruct()
{
    Super::NativeConstruct();
    SetVisibility(ESlateVisibility::Hidden);
}

void UDamageUI::DamageTimerExpired()
{
    SetVisibility(ESlateVisibility::Hidden);
    CurrentDamage = 0;
}
