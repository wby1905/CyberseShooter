// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Announcement.generated.h"

class UTextBlock;

UCLASS()
class CYBERSE_API UAnnouncement : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetCountdownText(const FString& Text);
	void SetAnnouncementText(const FString& Text);
	void SetInfoText(const FString& Text);
private:
	UPROPERTY(meta = (BindWidget))
        TObjectPtr<UTextBlock> AnnouncementText;
	UPROPERTY(meta = (BindWidget))
		TObjectPtr<UTextBlock> WarmupTime;
	UPROPERTY(meta = (BindWidget))
		TObjectPtr<UTextBlock> InfoText;
};
