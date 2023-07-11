// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/CyberseHUD.h"
#include "GameFramework/PlayerController.h"

#include "HUD/CharacterOverlay.h"
#include "HUD/Announcement.h"

void ACyberseHUD::DrawHUD()
{
    Super::DrawHUD();

    FVector2D ViewportSize;
    if (GEngine)
    {
        GEngine->GameViewport->GetViewportSize(ViewportSize);
        const FVector2D Center(ViewportSize.X / 2, ViewportSize.Y / 2);

        float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpreadMultiplier;

        if (HUDPackage.CrosshairsCenter)
        {
            DrawCrosshair(HUDPackage.CrosshairsCenter, Center, FVector2D::ZeroVector, HUDPackage.CrosshairColor);
        }
        if (HUDPackage.CrosshairsLeft)
        {
            DrawCrosshair(HUDPackage.CrosshairsLeft, Center, FVector2D(-SpreadScaled, 0.f), HUDPackage.CrosshairColor);
        }
        if (HUDPackage.CrosshairsRight)
        {
            DrawCrosshair(HUDPackage.CrosshairsRight, Center, FVector2D(SpreadScaled, 0.f), HUDPackage.CrosshairColor);
        }
        if (HUDPackage.CrosshairsTop)
        {
            DrawCrosshair(HUDPackage.CrosshairsTop, Center, FVector2D(0.f, -SpreadScaled), HUDPackage.CrosshairColor);
        }
        if (HUDPackage.CrosshairsBottom)
        {
            DrawCrosshair(HUDPackage.CrosshairsBottom, Center, FVector2D(0.f, SpreadScaled), HUDPackage.CrosshairColor);
        }
    }
}

void ACyberseHUD::AddCharacterOverlay()
{
    if (Announcement && Announcement->IsInViewport())
    {
        Announcement->RemoveFromViewport();
    }
    if (CharacterOverlay == nullptr)
    {
        APlayerController* PlayerController = GetOwningPlayerController();
        if (PlayerController)
        {
            if (CharacterOverlayClass && CharacterOverlay == nullptr)
                CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass);
        }
    }

    if (CharacterOverlay)
    {
        if (!CharacterOverlay->IsInViewport())
            CharacterOverlay->AddToViewport();
        CharacterOverlay->SetVisibility(ESlateVisibility::Visible);

    }
}

void ACyberseHUD::AddAnnouncement()
{
    if (CharacterOverlay && CharacterOverlay->IsInViewport())
    {
        CharacterOverlay->RemoveFromViewport();
    }
    if (Announcement == nullptr)
    {
        APlayerController* PlayerController = GetOwningPlayerController();
        if (PlayerController)
        {
            if (AnnouncementClass && Announcement == nullptr)
                Announcement = CreateWidget<UAnnouncement>(PlayerController, AnnouncementClass);
        }

    }

    if (Announcement)
    {
        if (!Announcement->IsInViewport())
            Announcement->AddToViewport();
        Announcement->SetVisibility(ESlateVisibility::Visible);
    }

}

void ACyberseHUD::BeginPlay()
{
    Super::BeginPlay();
    APlayerController* PlayerController = GetOwningPlayerController();
    if (PlayerController)
    {
        if (CharacterOverlayClass && CharacterOverlay == nullptr)
            CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass);
        if (AnnouncementClass && Announcement == nullptr)
            Announcement = CreateWidget<UAnnouncement>(PlayerController, AnnouncementClass);
    }
}


void ACyberseHUD::DrawCrosshair(UTexture2D* Texture, FVector2D Center, FVector2D Spread, FLinearColor Color)
{
    const float TextureWidth = Texture->GetSizeX();
    const float TextureHeight = Texture->GetSizeY();
    const FVector2D TextureDrawPoint(
        Center.X - TextureWidth / 2.f + Spread.X, 
        Center.Y - TextureHeight / 2.f + Spread.Y
    );

    DrawTexture(Texture, TextureDrawPoint.X, TextureDrawPoint.Y, TextureWidth, TextureHeight, 0.f, 0.f, 1.f, 1.f, Color);
}
