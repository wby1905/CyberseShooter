// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/CharacterOverlay.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/BackgroundBlur.h"
#include "Components/ScrollBox.h"
#include "Components/HorizontalBox.h"
#include "Components/VerticalBox.h"
#include "Components/EditableTextBox.h"

#include "Controller/BlasterPlayerController.h"

void UCharacterOverlay::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (bIsBleedingOut && BleedoutBar && HealthBar)
    {
        float BleedoutPercent = FMath::FInterpTo(BleedoutBar->GetPercent(), HealthBar->GetPercent(), InDeltaTime, 5.f);
        BleedoutBar->SetPercent(BleedoutPercent);
    }

}

void UCharacterOverlay::NativeConstruct()
{
    Super::NativeConstruct();
}

void UCharacterOverlay::SetHealth(float Health, float MaxHealth)
{
    Health = FMath::Clamp(Health, 0.f, MaxHealth - 0.00001f);

    if (HealthText)
    {
        HealthText->SetText(FText::FromString(FString::Printf(TEXT("%.0f/%.0f"), Health, MaxHealth)));
    }

    if (HealthBar)
    {
        HealthBar->SetPercent(Health / MaxHealth);

    }

    StartBleedoutTimer();
}

void UCharacterOverlay::SetShield(float Shield, float MaxShield)
{
    Shield = FMath::Clamp(Shield, 0.f, MaxShield - 0.00001f);

    if (ShieldBar)
    {
        ShieldBar->SetPercent(Shield / MaxShield);
    }
}

void UCharacterOverlay::SetScore(float Score)
{
    if (ScoreAmount)
    {
        ScoreAmount->SetText(FText::FromString(FString::Printf(TEXT("%.0f"), Score)));
    }
}

void UCharacterOverlay::SetDeathCount(int32 DeathCount)
{
    if (DeathAmount)
    {
        DeathAmount->SetText(FText::FromString(FString::Printf(TEXT("%d"), DeathCount)));
    }
}

void UCharacterOverlay::ToggleBlur(bool bIsBlur)
{
    if (Blur == nullptr) return;
    if (bIsBlur)
    {
        Blur->SetVisibility(ESlateVisibility::Visible);
    }
    else
    {
        Blur->SetVisibility(ESlateVisibility::Hidden);
    }
}

void UCharacterOverlay::SetRespawnTime(float Time)
{
    if (RespawnTime)
    {
        RespawnTime->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), Time)));
    }
}

void UCharacterOverlay::SetKilledByName(const FString& Name)
{
    if (KilledByName)
    {
        KilledByName->SetText(FText::FromString(Name));
    }
}

void UCharacterOverlay::SetWeaponAmmo(int32 WeaponAmmo)
{
    if (WeaponAmmoAmount)
    {
        WeaponAmmoAmount->SetText(FText::FromString(FString::Printf(TEXT("%d"), WeaponAmmo)));
    }
}

void UCharacterOverlay::SetTotalAmmo(int32 TotalAmmo)
{
    if (TotalAmmoAmount)
    {
        TotalAmmoAmount->SetText(FText::FromString(FString::Printf(TEXT("%d"), TotalAmmo)));
    }
}

void UCharacterOverlay::SetWeaponName(const FString& Name)
{
    if (WeaponName)
    {
        WeaponName->SetText(FText::FromString(Name));
    }
}

void UCharacterOverlay::SetMatchCountdownText(const FString& Text)
{
   if (MatchCountdownText)
    {
        MatchCountdownText->SetText(FText::FromString(Text));
    }
}

void UCharacterOverlay::SetGrenadeAmount(int32 Amount)
{
    if (GrenadeAmount)
    {
        GrenadeAmount->SetText(FText::FromString(FString::Printf(TEXT("%d"), Amount)));
    }
}

void UCharacterOverlay::SetPingAmount(int32 Amount, FLinearColor Color)
{
    if (PingAmount)
    {
        PingAmount->SetText(FText::FromString(FString::Printf(TEXT("%dms"), Amount)));
        PingAmount->SetColorAndOpacity(Color);
    }

    if (PingText)
    {
        PingText->SetColorAndOpacity(Color);
    }
}

void UCharacterOverlay::AddKillFeed(const FString& KillerName, const FString& VictimName, const FString& Mid)
{
    if (KillFeed && KillFeedItemClass)
    {
       UUserWidget* KillFeedItem = CreateWidget<UUserWidget>(GetWorld(), KillFeedItemClass);
        if (KillFeedItem)
        {
            UTextBlock* KillerNameText = Cast<UTextBlock>(KillFeedItem->GetWidgetFromName(TEXT("KillerName")));
            UTextBlock* MidText = Cast<UTextBlock>(KillFeedItem->GetWidgetFromName(TEXT("MidText")));
            UTextBlock* VictimNameText = Cast<UTextBlock>(KillFeedItem->GetWidgetFromName(TEXT("VictimName")));

            if (KillerNameText)
            {
                KillerNameText->SetText(FText::FromString(KillerName));
            }

            if (MidText)
            {
                MidText->SetText(FText::FromString(Mid));
            }

            if (VictimNameText)
            {
                VictimNameText->SetText(FText::FromString(VictimName));
            }

            KillFeed->AddChild(KillFeedItem);
            KillFeed->ScrollToEnd();
        }
    }

}

void UCharacterOverlay::AddChatMessage(const FString& Name, const FText& Message)
{
    if (ChatBox && ChatClass && ChatInput)
    {
        if (Message.IsEmpty())
        {
            return;
        }
        UUserWidget* ChatItem = CreateWidget<UUserWidget>(GetWorld(), ChatClass);
        if (ChatItem)
        {
            UTextBlock* ChatName = Cast<UTextBlock>(ChatItem->GetWidgetFromName(TEXT("ChatName")));
            if (ChatName)
            {
                ChatName->SetText(FText::FromString(Name));
            }

            UTextBlock* ChatText = Cast<UTextBlock>(ChatItem->GetWidgetFromName(TEXT("ChatText")));
            if (ChatText)
            {
                ChatText->SetText(Message);
            }

            ChatBox->AddChild(ChatItem);
            ChatBox->ScrollToEnd();
        }


    }
    if (ChatBox)
    {
        ChatBox->SetVisibility(ESlateVisibility::Visible);
        if (ChatTimerHandle.IsValid())
        {
            GetWorld()->GetTimerManager().ClearTimer(ChatTimerHandle);
        }

        GetWorld()->GetTimerManager().SetTimer(ChatTimerHandle, this, &UCharacterOverlay::ChatTimerExpired, ChatVisibleTime, false);
    }
}

void UCharacterOverlay::ToggleChatZone(bool bEnabled)
{
    if (ChatZone == nullptr) return;
    if (ChatTimerHandle.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(ChatTimerHandle);
    }
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC == nullptr) return;
    if (bEnabled)
    {
        if (ChatBox)
        {
            ChatBox->SetVisibility(ESlateVisibility::Visible);
        }

        FInputModeGameAndUI InputMode;
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockOnCapture);
        InputMode.SetWidgetToFocus(TakeWidget());
        PC->SetInputMode(InputMode);
        ChatInput->SetFocus();
    }
    else
    {
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        GetWorld()->GetTimerManager().SetTimer(ChatTimerHandle, this, &UCharacterOverlay::ChatTimerExpired, ChatVisibleTime, false);
    }
}

void UCharacterOverlay::StartTimerBlink()
{
    if (MatchCountdownText)
        MatchCountdownText->SetColorAndOpacity(FLinearColor::Red);
    if (TimerBlink)
        PlayAnimation(TimerBlink);
}

void UCharacterOverlay::ToggleHighPingBlink(bool bEnabled)
{
    if (HighPing && HighPingImage)
    {
        if (bEnabled)
        {
            HighPingImage->SetOpacity(1.f);
            PlayAnimation(HighPing, 0.f, 3);
        }
        else
        {
            if (IsAnimationPlaying(HighPing))
                StopAnimation(HighPing);
            HighPingImage->SetOpacity(0.f);
        }
    }
}

bool UCharacterOverlay::Initialize()
{
    if (!Super::Initialize()) return false;

    if (ChatInput)
    {
        UE_LOG(LogTemp, Warning, TEXT("Chat input initialized"))
        ChatInput->OnTextCommitted.AddDynamic(this, &UCharacterOverlay::OnChatInputCommitted);
    }
    return true;
}


void UCharacterOverlay::ChatTimerExpired()
{
    if (ChatBox)
        ChatBox->SetVisibility(ESlateVisibility::Hidden);
}

void UCharacterOverlay::OnChatInputCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
    UE_LOG(LogTemp, Warning, TEXT("Chat input committed"))
    ABlasterPlayerController* PC = Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController());
    if (PC == nullptr) return;
    PC->OnChatInputCommited(Text, CommitMethod);
}

void UCharacterOverlay::StartBleedoutTimer()
{
    bIsBleedingOut = false;
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(BleedoutTimerHandle);
        GetWorld()->GetTimerManager().SetTimer(BleedoutTimerHandle, this, &UCharacterOverlay::BleedoutTimerExpired, BleedoutTime);
    }
}

void UCharacterOverlay::BleedoutTimerExpired()
{
    bIsBleedingOut = true;
}
