// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHighPingDelegate, bool, bHighPing);

UCLASS()
class CYBERSE_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	virtual void ReceivedPlayer() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void SetupInputComponent() override;

	void OnChatInputCommited(const FText& Text, ETextCommit::Type CommitMethod);

	virtual float GetServerTime() const;
	void OnMatchStateSet(FName State);

	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDShield(float Shield, float MaxShield);
	void SetHUDScore(float Score);
	void SetHUDDeathCount(int32 DeathCount);
	void ToggleHUDBlur(bool bIsBlur);
	void SetHUDRespawnTimer(float RespawnTimer);
	void SetHUDKilledBy(const FString& KillerName);
	void SetHUDWeaponAmmo(int32 CurrentAmmo);
	void SetHUDTotalAmmo(int32 TotalAmmo);
	void SetHUDWeaponName(const FString& WeaponName);
	void SetHUDMatchTimer(float CountdownTime);
	void SetHUDAnnouncementTimer(float CountdownTime);
	void SetHUDGrenadeAmount(int32 Amount);
	void ToggleHUDHighPingBlink(bool bEnabled);
	void SetHUDPingAmount(float Amount);
	void ToggleHUDChatZone(bool bEnabled);
	void AddHUDChatMessage(const FString& Name, const FText& Message);

	UFUNCTION(Client, Reliable)
		void ClientSetHUDKillFeed(const FString& KillerName, const FString& VictimName, const FString& WeaponName);

	float SingleTripTime = 0.f;
	FHighPingDelegate OnHighPing;


	FString GetChatInput();
protected:
	virtual void BeginPlay() override;

	void PollInit();

	/**
	* Sync time between client and server
	*/
	float ClientServerDelta = 0.f;
	float TimeSyncElapsed = 0.f;
	UPROPERTY(EditDefaultsOnly, Category = Time)
		float TimeSyncFreq = 5.f;

	void SyncTime(float DeltaSeconds);
	// Requests the current server time, passing in the client's time when it is sent
	UFUNCTION(Server, Reliable)
        void ServerSyncTime(float TimeOfClientReq);
	// Reports the current server time to the client in response to ServerRequestServerTime
	UFUNCTION(Client, Reliable)
		void ClientReportServerTime(float TimeOfClientReq, float TimeServerReceivedClientReq);


	UFUNCTION(Server, Reliable)
		void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
		void ClientJoinMidgame(FName StateOfMatch, float Warmup, float Match, float CoolDown, float LevelStart);

private:
	UPROPERTY()
		TObjectPtr<class ACyberseHUD> CyberseHUD;
	UPROPERTY()
		TObjectPtr<class UCharacterOverlay> CharacterOverlay;
	UPROPERTY()
		TObjectPtr<class UAnnouncement> Announcement;
	UPROPERTY()
		TObjectPtr<class ACyberseGameMode> GameMode;

	// Cache the data before it is initialized
	bool bInitializeHealth = false;
	float HUDHealth;
	float HUDMaxHealth;
	bool bInitializeShield = false;
	float HUDShield;
	float HUDMaxShield;
	bool bInitializeScore = false;
	float HUDScore;
	bool bInitializeDeathCount = false;
    int32 HUDDeathCount;
	bool bInitializeMatchTimer = false;
	float HUDMatchTimer;
	bool bInitializeAnnouncementTimer = false;
	float HUDAnnouncementTimer;
	bool bInitializeGrenadeAmount = false;
	int32 HUDGrenadeAmount;
	bool bInitializeWeaponAmmo = false;
	int32 HUDWeaponAmmo;
	bool bInitializeTotalAmmo = false;
	int32 HUDTotalAmmo;
	bool bInitializeWeaponName = false;
	FString HUDWeaponName;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
		FName MatchState;

	float LevelStartingTime = 0.f;
	float MatchTime = 0.f;
	float WarmupTime = 0.f;
	float CoolDownTime = 0.f;
	int32 CountdownInt = 0;


	void UpdateMatchTimer();
	TObjectPtr<UCharacterOverlay> CheckGetHUDOverlay();
	TObjectPtr<UAnnouncement> CheckGetHUDAnnouncement();

	UFUNCTION()
		void OnRep_MatchState();

	void HandleCoolDown();

	/**
	* High ping
	*/
	float HighPingRunningTime = 0.f;
	float PingAnimRunningTime = 0.f;
	bool bIsHighPing = false;
	UPROPERTY(EditAnywhere)
		float HighPingBlinkDuration = 10.f;
	UPROPERTY(EditAnywhere)
		float CheckPingFreq = 10.f;
	UPROPERTY(EditAnywhere)
		float HighPingThreshold = 50.f;

	void CheckPing(float DeltaSeconds);

	UFUNCTION(Server, Reliable)
		void ServerReportPingStatus(bool bHighPing);

	// Chat

	UPROPERTY(EditDefaultsOnly, Category = Input)
		TObjectPtr<class UInputAction> ChatAction;

	void ChatButton();

	UFUNCTION(Server, Reliable)
		void ServerSendChatMessage(const FText& Message);

	UFUNCTION(Client, Reliable)
		void ClientSendChatMessage(const FString& Name, const FText& Message);



};
