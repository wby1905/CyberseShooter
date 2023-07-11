// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "Menu.generated.h"

class UButton;

UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
		void MenuSetup(TSoftObjectPtr<UWorld> LobbyLevel, int32 NumberOfPublicConnections = 4, FString TypeOfMatch = FString(TEXT("FreeForAll")));
	void MenuSetup(FString LobbyPath = FString(TEXT("/Game/ThirdPerson/Maps/Lobby")), int32 NumberOfPublicConnections = 4, FString TypeOfMatch = FString(TEXT("FreeForAll")));
protected:

	virtual bool Initialize() override;
	virtual void NativeDestruct() override;

	/**
	* Callbacks for the custom delegates on the MultiplayerSessionsSubsystem
	*/
	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
    void OnJoinSession(EOnJoinSessionCompleteResult::Type Result, FString Address);
	UFUNCTION()
		void OnCreateSession(bool bWasSuccessful);
	UFUNCTION()
		void OnDestroySession(bool bWasSuccessful);
	UFUNCTION()
		void OnStartSession(bool bWasSuccessful);

private:
	UPROPERTY(meta = (BindWidget))
		TObjectPtr<UButton> HostButton;
	UPROPERTY(meta = (BindWidget))
        TObjectPtr<UButton> JoinButton;

	UFUNCTION()
	void HostButtonClicked();

	UFUNCTION()
    void JoinButtonClicked();

	void MenuTearDown();

	// The subsystem to handle all functions related to multiplayer sessions
	TObjectPtr<class UMultiplayerSessionsSubsystem> MultiplayerSessionsSubsystem;

	int32 NumPublicConnections{ 4 };
	FString MatchType{ TEXT("FreeForAll") };
	FString PathToLobby{ TEXT("") };
};
