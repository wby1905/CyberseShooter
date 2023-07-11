// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"

class UProgressBar;
class UTextBlock;

UCLASS()
class CYBERSE_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeConstruct() override;

	void SetHealth(float Health, float MaxHealth);
	void SetShield(float Shield, float MaxShield);
	void SetScore(float Score);
	void SetDeathCount(int32 DeathCount);
	void ToggleBlur(bool bIsBlur);
	void SetRespawnTime(float Time);
	void SetKilledByName(const FString& Name);
	void SetWeaponAmmo(int32 WeaponAmmo);
	void SetTotalAmmo(int32 TotalAmmo);
	void SetWeaponName(const FString& Name);
	UFUNCTION(BlueprintCallable)
		void SetMatchCountdownText(const FString& Text);
	void SetGrenadeAmount(int32 Amount);
	void SetPingAmount(int32 Amount, FLinearColor Color);
	void AddKillFeed(const FString& KillerName, const FString& VictimName, const FString& WeaponName);
	void AddChatMessage(const FString& Name, const FText& Message);
	void ToggleChatZone(bool bEnabled);

	void StartTimerBlink();
	void ToggleHighPingBlink(bool bEnabled);

	FORCEINLINE TObjectPtr<class UEditableTextBox> GetChatInput() const { return ChatInput; }
protected:
	virtual bool Initialize() override;

private:
	UPROPERTY(meta = (BindWidget))
		TObjectPtr<UTextBlock> HealthText;
	UPROPERTY(meta = (BindWidget))
		TObjectPtr<UProgressBar> HealthBar;
	UPROPERTY(meta = (BindWidget))
		TObjectPtr<UProgressBar> BleedoutBar;
	UPROPERTY(meta = (BindWidget))
		TObjectPtr<UProgressBar> ShieldBar;
	UPROPERTY(meta = (BindWidget))
        TObjectPtr<UTextBlock> ScoreAmount;
	UPROPERTY(meta = (BindWidget))
        TObjectPtr<UTextBlock> DeathAmount;
	UPROPERTY(meta = (BindWidget))
		TObjectPtr<class UBackgroundBlur> Blur;
	UPROPERTY(meta = (BindWidget))
		TObjectPtr<UTextBlock> RespawnTime;
	UPROPERTY(meta = (BindWidget))
		TObjectPtr<UTextBlock> KilledByName;
	UPROPERTY(meta = (BindWidget))
        TObjectPtr<class UScrollBox> KillFeed;
	UPROPERTY(meta = (BindWidget))
		TObjectPtr<UTextBlock> WeaponAmmoAmount;
	UPROPERTY(meta = (BindWidget))
		TObjectPtr<UTextBlock> TotalAmmoAmount;
	UPROPERTY(meta = (BindWidget))
		TObjectPtr<UTextBlock> WeaponName;
	UPROPERTY(meta = (BindWidget))
		TObjectPtr<UTextBlock> MatchCountdownText;
	UPROPERTY(meta = (BindWidget))
		TObjectPtr<UTextBlock> GrenadeAmount;
	UPROPERTY(meta = (BindWidget))
		TObjectPtr<class UImage> HighPingImage;
	UPROPERTY(meta = (BindWidget))
		TObjectPtr<UTextBlock> PingText;
	UPROPERTY(meta = (BindWidget))
		TObjectPtr<UTextBlock> PingAmount;
	UPROPERTY(meta = (BindWidget))
		TObjectPtr<class UVerticalBox> ChatZone;
	UPROPERTY(meta = (BindWidget))
		TObjectPtr<UScrollBox> ChatBox;
	UPROPERTY(meta = (BindWidget))
		TObjectPtr<UEditableTextBox> ChatInput;


	UPROPERTY(meta = (BindWidgetAnim), Transient)
        TObjectPtr<UWidgetAnimation> TimerBlink;
	UPROPERTY(meta = (BindWidgetAnim), Transient)
		TObjectPtr<UWidgetAnimation> HighPing;

	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<UUserWidget> KillFeedItemClass;

	/**
	* Chat
	*/
	UPROPERTY(EditDefaultsOnly)
		float ChatVisibleTime = 5.0f;
	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<UUserWidget> ChatClass;
	FTimerHandle ChatTimerHandle;
	void ChatTimerExpired();
	UFUNCTION()
		void OnChatInputCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	/**
	* Bleedout
	*/
	UPROPERTY(EditDefaultsOnly)
		float BleedoutTime = 0.5f;
	bool bIsBleedingOut = false;

	FTimerHandle BleedoutTimerHandle;
	void StartBleedoutTimer();
	void BleedoutTimerExpired();
};
