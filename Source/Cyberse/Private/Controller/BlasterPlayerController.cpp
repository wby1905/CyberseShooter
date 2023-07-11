// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/BlasterPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerState.h"
#include "Components/EditableTextBox.h"
#include "EnhancedInputComponent.h"

#include "HUD/CyberseHUD.h"
#include "HUD/CharacterOverlay.h"
#include "HUD/Announcement.h"
#include "Character/BlasterCharacter.h"
#include "GameMode/CyberseGameMode.h"
#include "State/CyberseGameState.h"
#include "State/BlasterPlayerState.h"

void ABlasterPlayerController::ReceivedPlayer()
{
    Super::ReceivedPlayer();
    if (IsLocalController())
    {
        ServerSyncTime(GetWorld()->GetTimeSeconds());
        ServerCheckMatchState();
    }
}

void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    CyberseHUD = CyberseHUD == nullptr ? Cast<ACyberseHUD>(GetHUD()) : CyberseHUD;
    ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(InPawn);
    if (BlasterCharacter)
    {
        SetHUDHealth(BlasterCharacter->GetHealth(), BlasterCharacter->GetMaxHealth());
        SetHUDShield(BlasterCharacter->GetShield(), BlasterCharacter->GetMaxShield());
    }

}

void ABlasterPlayerController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (IsLocalController())
    {
        PollInit();
        UpdateMatchTimer();
        SyncTime(DeltaSeconds);
        CheckPing(DeltaSeconds);
    }
}


void ABlasterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ABlasterPlayerController, MatchState);
}

void ABlasterPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
    {
        // Chat
        EnhancedInputComponent->BindAction(ChatAction, ETriggerEvent::Triggered, this, &ABlasterPlayerController::ChatButton);
    }
}

void ABlasterPlayerController::OnChatInputCommited(const FText& Text, ETextCommit::Type CommitMethod)
{
    if (!IsLocalController()) return;
    if (CommitMethod != ETextCommit::OnCleared)
    {
        ABlasterPlayerState* BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
        if (BlasterPlayerState == nullptr) return;
        if (!Text.IsEmpty())
        {
            ServerSendChatMessage(Text);
        }
        // Locally add the message to the chat window first
        AddHUDChatMessage(BlasterPlayerState->GetPlayerName(), Text);
    }
    ToggleHUDChatZone(false);
}

float ABlasterPlayerController::GetServerTime() const
{
    return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void ABlasterPlayerController::OnMatchStateSet(FName State)
{
    MatchState = State;
    OnRep_MatchState();
}

void ABlasterPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
    CharacterOverlay = CharacterOverlay == nullptr ? CheckGetHUDOverlay() : CharacterOverlay;
    if (CharacterOverlay)
        CharacterOverlay->SetHealth(Health, MaxHealth);
    else
    {
        bInitializeHealth = true;
        HUDHealth = Health;
        HUDMaxHealth = MaxHealth;
    }
}

void ABlasterPlayerController::SetHUDShield(float Shield, float MaxShield)
{
    CharacterOverlay = CharacterOverlay == nullptr ? CheckGetHUDOverlay() : CharacterOverlay;
    if (CharacterOverlay)
        CharacterOverlay->SetShield(Shield, MaxShield);
    else
    {
        bInitializeShield = true;
        HUDShield = Shield;
        HUDMaxShield = MaxShield;
    }
}

void ABlasterPlayerController::SetHUDScore(float Score)
{
    CharacterOverlay = CharacterOverlay == nullptr ? CheckGetHUDOverlay() : CharacterOverlay;
    if (CharacterOverlay)
        CharacterOverlay->SetScore(Score);
    else
    {
        bInitializeScore = true;
        HUDScore = Score;
    }
}

void ABlasterPlayerController::SetHUDDeathCount(int32 DeathCount)
{
    CharacterOverlay = CharacterOverlay == nullptr ? CheckGetHUDOverlay() : CharacterOverlay;
    if (CharacterOverlay)  
        CharacterOverlay->SetDeathCount(DeathCount);
    else
    {
        bInitializeDeathCount = true;
        HUDDeathCount = DeathCount;
    }
}

void ABlasterPlayerController::ToggleHUDBlur(bool bIsBlur)
{
    CharacterOverlay = CharacterOverlay == nullptr ? CheckGetHUDOverlay() : CharacterOverlay;
    if (CharacterOverlay)
        CharacterOverlay->ToggleBlur(bIsBlur);
}

void ABlasterPlayerController::SetHUDRespawnTimer(float RespawnTimer)
{
    CharacterOverlay = CharacterOverlay == nullptr ? CheckGetHUDOverlay() : CharacterOverlay;
    if (CharacterOverlay)
        CharacterOverlay->SetRespawnTime(RespawnTimer);
}

void ABlasterPlayerController::SetHUDKilledBy(const FString& KillerName)
{
    CharacterOverlay = CharacterOverlay == nullptr ? CheckGetHUDOverlay() : CharacterOverlay;
    if (CharacterOverlay)
        CharacterOverlay->SetKilledByName(KillerName);
}

void ABlasterPlayerController::SetHUDWeaponAmmo(int32 CurrentAmmo)
{
    CharacterOverlay = CharacterOverlay == nullptr ? CheckGetHUDOverlay() : CharacterOverlay;
    if (CharacterOverlay)
        CharacterOverlay->SetWeaponAmmo(CurrentAmmo);
    else
    {
        bInitializeWeaponAmmo = true;
        HUDWeaponAmmo = CurrentAmmo;
    }
}

void ABlasterPlayerController::SetHUDTotalAmmo(int32 TotalAmmo)
{
    CharacterOverlay = CharacterOverlay == nullptr ? CheckGetHUDOverlay() : CharacterOverlay;
    if (CharacterOverlay)
        CharacterOverlay->SetTotalAmmo(TotalAmmo);
    else
    {
        bInitializeTotalAmmo = true;
        HUDTotalAmmo = TotalAmmo;
    }
}

void ABlasterPlayerController::SetHUDWeaponName(const FString& WeaponName)
{
    CharacterOverlay = CharacterOverlay == nullptr ? CheckGetHUDOverlay() : CharacterOverlay;
    if (CharacterOverlay)
        CharacterOverlay->SetWeaponName(WeaponName);
    else
    {
        bInitializeWeaponName = true;
        HUDWeaponName = WeaponName;
    }
}

void ABlasterPlayerController::SetHUDMatchTimer(float CountdownTime)
{
    CharacterOverlay = CharacterOverlay == nullptr ? CheckGetHUDOverlay() : CharacterOverlay;
    if (CharacterOverlay)
    {
        if (CountdownTime < 0.1f)
        {
            CharacterOverlay->SetMatchCountdownText(FString::Printf(TEXT("%d"), CountdownTime));
            return;
        }
        if (CountdownTime < 30.f)
        {
            CharacterOverlay->StartTimerBlink();
        }
        int32 Minutes = FMath::FloorToInt(CountdownTime / 60);
        int32 Seconds = CountdownTime - Minutes * 60;
        FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
        CharacterOverlay->SetMatchCountdownText(CountdownText);
    }
    else
    {
        bInitializeMatchTimer = true;
        HUDMatchTimer = CountdownTime;
    }
}

void ABlasterPlayerController::SetHUDAnnouncementTimer(float CountdownTime)
{
    Announcement = Announcement == nullptr ? CheckGetHUDAnnouncement() : Announcement;
    if (Announcement)
    {
        if (CountdownTime < 0.1f)
        {
            Announcement->SetCountdownText(FString());
            return;
        }
        int32 Minutes = FMath::FloorToInt(CountdownTime / 60);
        int32 Seconds = CountdownTime - Minutes * 60;
        FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
        Announcement->SetCountdownText(CountdownText);
    }
    else
    {
        bInitializeAnnouncementTimer = true;
        HUDAnnouncementTimer = CountdownTime;
    }
}

void ABlasterPlayerController::SetHUDGrenadeAmount(int32 Amount)
{
    CharacterOverlay = CharacterOverlay == nullptr ? CheckGetHUDOverlay() : CharacterOverlay;
    if (CharacterOverlay)
        CharacterOverlay->SetGrenadeAmount(Amount);
    else
    {
        bInitializeGrenadeAmount = true;
        HUDGrenadeAmount = Amount;
    }
}

void ABlasterPlayerController::ToggleHUDHighPingBlink(bool bEnabled)
{
    CharacterOverlay = CharacterOverlay == nullptr ? CheckGetHUDOverlay() : CharacterOverlay;
    if (CharacterOverlay)
        CharacterOverlay->ToggleHighPingBlink(bEnabled);
}

void ABlasterPlayerController::SetHUDPingAmount(float Amount)
{
    CharacterOverlay = CharacterOverlay == nullptr ? CheckGetHUDOverlay() : CharacterOverlay;
    if (CharacterOverlay)
    {
        FLinearColor PingColor = FLinearColor::Green;
        if (Amount > HighPingThreshold)
            PingColor = FLinearColor::Red;
        else if (Amount > HighPingThreshold / 2)
            PingColor = FLinearColor::Yellow;
        CharacterOverlay->SetPingAmount(Amount, PingColor);
    }
}

void ABlasterPlayerController::ToggleHUDChatZone(bool bEnabled)
{
    CharacterOverlay = CharacterOverlay == nullptr ? CheckGetHUDOverlay() : CharacterOverlay;
    if (CharacterOverlay)
    {
        CharacterOverlay->ToggleChatZone(bEnabled);
        if (!bEnabled && CharacterOverlay->GetChatInput())
            CharacterOverlay->GetChatInput()->SetText(FText::FromString(TEXT("")));
    }
}

void ABlasterPlayerController::AddHUDChatMessage(const FString& Name, const FText& Message)
{
    CharacterOverlay = CharacterOverlay == nullptr ? CheckGetHUDOverlay() : CharacterOverlay;
    if (CharacterOverlay)
        CharacterOverlay->AddChatMessage(Name, Message);
}

void ABlasterPlayerController::ClientSetHUDKillFeed_Implementation(const FString& KillerName, const FString& VictimName, const FString& WeaponName)
{
    CharacterOverlay = CharacterOverlay == nullptr ? CheckGetHUDOverlay() : CharacterOverlay;
    if (CharacterOverlay)
        CharacterOverlay->AddKillFeed(KillerName, VictimName, WeaponName);
}

FString ABlasterPlayerController::GetChatInput()
{
    CharacterOverlay = CharacterOverlay == nullptr ? CheckGetHUDOverlay() : CharacterOverlay;
    if (CharacterOverlay && CharacterOverlay->GetChatInput())
        return CharacterOverlay->GetChatInput()->GetText().ToString();
    return FString();
}

void ABlasterPlayerController::BeginPlay()
{
    Super::BeginPlay();
    GameMode = Cast<ACyberseGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
    ServerCheckMatchState();
}

void ABlasterPlayerController::PollInit()
{
    if (CharacterOverlay == nullptr)
    {
        if (CyberseHUD && CyberseHUD->GetCharacterOverlay())
        {
            CharacterOverlay = CyberseHUD->GetCharacterOverlay();
            if (CharacterOverlay)
            {
                if (bInitializeHealth)
                    SetHUDHealth(HUDHealth, HUDMaxHealth);
                if (bInitializeShield)
                    SetHUDShield(HUDShield, HUDMaxShield);
                if (bInitializeScore)
                    SetHUDScore(HUDScore);
                if (bInitializeDeathCount)
                    SetHUDDeathCount(HUDDeathCount);
                if (bInitializeMatchTimer)
                    SetHUDMatchTimer(HUDMatchTimer);
                if (bInitializeGrenadeAmount)
                    SetHUDGrenadeAmount(HUDGrenadeAmount);
                if (bInitializeWeaponAmmo)
                    SetHUDWeaponAmmo(HUDWeaponAmmo);
                if (bInitializeTotalAmmo)
                    SetHUDTotalAmmo(HUDTotalAmmo);
                if (bInitializeWeaponName)
                    SetHUDWeaponName(HUDWeaponName);
            }
        }
    }   
    if (Announcement == nullptr)
    {
        if (CyberseHUD && CyberseHUD->GetAnnouncement())
        {
            Announcement = CyberseHUD->GetAnnouncement();
            if (Announcement)
            {
                if (bInitializeAnnouncementTimer)
                    SetHUDAnnouncementTimer(HUDAnnouncementTimer);
            }
        }
    }
}

void ABlasterPlayerController::SyncTime(float DeltaSeconds)
{
    TimeSyncElapsed += DeltaSeconds;
    if (TimeSyncElapsed > TimeSyncFreq)
    {
        ServerSyncTime(GetWorld()->GetTimeSeconds());
        ServerCheckMatchState();
        TimeSyncElapsed = 0.f;
    }
}

void ABlasterPlayerController::ServerSyncTime_Implementation(float TimeOfClientReq)
{
    float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
    ClientReportServerTime(TimeOfClientReq, ServerTimeOfReceipt);
}

void ABlasterPlayerController::ClientReportServerTime_Implementation(float TimeOfClientReq, float TimeServerReceivedClientReq)
{
    float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientReq;
    SingleTripTime = RoundTripTime / 2.f;
    float ServerTime = TimeServerReceivedClientReq + SingleTripTime;
    ClientServerDelta = ServerTime - GetWorld()->GetTimeSeconds();
    
}

void ABlasterPlayerController::ServerCheckMatchState_Implementation()
{
    GameMode = GameMode == nullptr ? Cast<ACyberseGameMode>(UGameplayStatics::GetGameMode(GetWorld())) : GameMode;
    if (GameMode)
    {
        WarmupTime = GameMode->WarmupTime;
        MatchTime = GameMode->MatchTime;
        CoolDownTime = GameMode->CoolDownTime;
        LevelStartingTime = GameMode->LevelStartingTime;
        MatchState = GameMode->GetMatchState();
        ClientJoinMidgame(MatchState, WarmupTime, MatchTime, CoolDownTime, LevelStartingTime);
    }

}

void ABlasterPlayerController::ClientJoinMidgame_Implementation(FName StateOfMatch, float Warmup, float Match, float CoolDown, float LevelStart)
{
    MatchState = StateOfMatch;
    WarmupTime = Warmup;
    MatchTime = Match;
    CoolDownTime = CoolDown;
    LevelStartingTime = LevelStart;
    OnMatchStateSet(MatchState);
    CyberseHUD = CyberseHUD == nullptr ? Cast<ACyberseHUD>(GetHUD()) : CyberseHUD;
    if (CyberseHUD && MatchState == MatchState::WaitingToStart)
    {
        CyberseHUD->AddAnnouncement();
    }
}

TObjectPtr<UCharacterOverlay> ABlasterPlayerController::CheckGetHUDOverlay()
{
    CyberseHUD = CyberseHUD == nullptr ? Cast<ACyberseHUD>(GetHUD()) : CyberseHUD;
    if (CyberseHUD && CyberseHUD->GetCharacterOverlay())
    {
        return CyberseHUD->GetCharacterOverlay();
    }
    return nullptr;
}

TObjectPtr<UAnnouncement> ABlasterPlayerController::CheckGetHUDAnnouncement()
{
    CyberseHUD = CyberseHUD == nullptr ? Cast<ACyberseHUD>(GetHUD()) : CyberseHUD;
    if (CyberseHUD && CyberseHUD->GetAnnouncement())
    {
        return CyberseHUD->GetAnnouncement();
    }
    return nullptr;
}

void ABlasterPlayerController::UpdateMatchTimer()
{
    float TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
    if (MatchState == MatchState::InProgress) TimeLeft = TimeLeft + MatchTime;
    else if (MatchState == MatchState::CoolDown) TimeLeft = TimeLeft + MatchTime + CoolDownTime;
    int32 SecondsLeft = FMath::CeilToInt(TimeLeft);

    if (HasAuthority())
    {
        GameMode = GameMode == nullptr ? Cast<ACyberseGameMode>(UGameplayStatics::GetGameMode(GetWorld())) : GameMode;
        if (GameMode)
        {
            SecondsLeft = FMath::CeilToInt(GameMode->GetCountdownTime());
        }
    }

    if (CountdownInt != SecondsLeft)
    {
        if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::CoolDown)
        {
            SetHUDAnnouncementTimer(SecondsLeft);
        }
        else if (MatchState == MatchState::InProgress)
        {
            SetHUDMatchTimer(SecondsLeft);
        }

    }
    CountdownInt = SecondsLeft;
}

void ABlasterPlayerController::OnRep_MatchState()
{
    if (!IsLocalController()) return;
    if (MatchState == MatchState::InProgress)
    {
        CyberseHUD = CyberseHUD == nullptr ? Cast<ACyberseHUD>(GetHUD()) : CyberseHUD;
        if (CyberseHUD)
        {
            CyberseHUD->AddCharacterOverlay();
        }
    }
    else if (MatchState == MatchState::CoolDown)
    {
        HandleCoolDown();
    }
    else if (MatchState == MatchState::WaitingToStart)
    {
        CyberseHUD = CyberseHUD == nullptr ? Cast<ACyberseHUD>(GetHUD()) : CyberseHUD;
        if (CyberseHUD)
        {
            CyberseHUD->AddAnnouncement();
        }
    }
}

void ABlasterPlayerController::HandleCoolDown()
{
    CyberseHUD = CyberseHUD == nullptr ? Cast<ACyberseHUD>(GetHUD()) : CyberseHUD;
    if (CyberseHUD)
    {
        CyberseHUD->AddAnnouncement();
    }
    Announcement = Announcement == nullptr ? CheckGetHUDAnnouncement() : Announcement;
    if (Announcement)
    {
        FString AnnouncementText(TEXT("New Match Starts In:"));
        Announcement->SetAnnouncementText(AnnouncementText);

        ACyberseGameState* CyberseGameState = GetWorld()->GetGameState<ACyberseGameState>();
        FString Winner;
        if (CyberseGameState)
        {
            Winner = CyberseGameState->GetTopScoringPlayerNames();
        }
        Announcement->SetInfoText(Winner);

    }
    ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetPawn());
    if (BlasterCharacter)
    {
        BlasterCharacter->ToggleDisableGameplay(true);
    }
}

void ABlasterPlayerController::CheckPing(float DeltaSeconds)
{
    HighPingRunningTime += DeltaSeconds;
    if (HighPingRunningTime > CheckPingFreq)
    {
        PlayerState = PlayerState == nullptr ? GetPlayerState<APlayerState>() : PlayerState;
        if (PlayerState)
        {
            float Ping = PlayerState->GetPingInMilliseconds();
            SetHUDPingAmount(Ping);
            if (Ping > HighPingThreshold)
            {
                ToggleHUDHighPingBlink(true);
                bIsHighPing = true;
            }
            else
            {
                ToggleHUDHighPingBlink(false);
                bIsHighPing = false;
            }
            ServerReportPingStatus(bIsHighPing);
        }
        HighPingRunningTime = 0.f;
    }

    if (bIsHighPing)
    {
        PingAnimRunningTime += DeltaSeconds;
        if (PingAnimRunningTime > HighPingBlinkDuration)
        {
            ToggleHUDHighPingBlink(true);
            PingAnimRunningTime = 0.f;
        }
    }

}

void ABlasterPlayerController::ServerReportPingStatus_Implementation(bool bHighPing)
{
    OnHighPing.Broadcast(bHighPing);
}


void ABlasterPlayerController::ChatButton()
{
    if (!IsLocalController()) return;
    ToggleHUDChatZone(true);
}

void ABlasterPlayerController::ServerSendChatMessage_Implementation(const FText& Message)
{
    ABlasterPlayerState* PS = GetPlayerState<ABlasterPlayerState>();
    if (PS)
    {
        const FString Name = PS->GetPlayerName();
        for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
        {
            ABlasterPlayerController* PC = Cast<ABlasterPlayerController>(*It);
            if (PC)
            {
                if (PC == this) continue;
                PC->ClientSendChatMessage(Name, Message);
            }
        }
    }
}

void ABlasterPlayerController::ClientSendChatMessage_Implementation(const FString& Name, const FText& Message)
{
    AddHUDChatMessage(Name, Message);
}

