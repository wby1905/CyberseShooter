// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/Announcement.h"

#include "Components/TextBlock.h"

void UAnnouncement::SetCountdownText(const FString& Text)
{
    if (WarmupTime)
    {
        WarmupTime->SetText(FText::FromString(Text));
    }
}

void UAnnouncement::SetAnnouncementText(const FString& Text)
{
    if (AnnouncementText)
    {
        AnnouncementText->SetText(FText::FromString(Text));
    }
}

void UAnnouncement::SetInfoText(const FString& Text)
{
    if (InfoText)
    {
        InfoText->SetText(FText::FromString(Text));
    }
}
