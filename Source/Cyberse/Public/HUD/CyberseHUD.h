// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CyberseHUD.generated.h"

class UTexture2D;
class UCharacterOverlay;
class UAnnouncement;

USTRUCT(BlueprintType)
struct FHUDPackage
{
    GENERATED_BODY()

public: 
	TObjectPtr<UTexture2D> CrosshairsCenter;
	TObjectPtr<UTexture2D> CrosshairsLeft;
	TObjectPtr<UTexture2D> CrosshairsRight;
	TObjectPtr<UTexture2D> CrosshairsTop;
	TObjectPtr<UTexture2D> CrosshairsBottom;
	float CrosshairSpreadMultiplier = 0.f;
	FLinearColor CrosshairColor = FLinearColor::White;
};

/**
 * 
 */
UCLASS()
class CYBERSE_API ACyberseHUD : public AHUD
{
	GENERATED_BODY()
public:
	virtual void DrawHUD() override;
	
	UFUNCTION(BlueprintCallable)
		void AddCharacterOverlay();
	UFUNCTION(BlueprintCallable)
		void AddAnnouncement();

	FORCEINLINE void SetHUDPackage(FHUDPackage Package) { HUDPackage = Package; }
	FORCEINLINE TObjectPtr<UCharacterOverlay> GetCharacterOverlay() { return CharacterOverlay; }
	FORCEINLINE TObjectPtr<UAnnouncement> GetAnnouncement() { return Announcement; }
protected:
	virtual void BeginPlay() override;
private:
	FHUDPackage HUDPackage;

	UPROPERTY(EditAnywhere)
		TSubclassOf<UCharacterOverlay> CharacterOverlayClass;
	UPROPERTY(VisibleAnywhere)
		TObjectPtr<UCharacterOverlay> CharacterOverlay;

	UPROPERTY(EditAnywhere)
		TSubclassOf<UAnnouncement> AnnouncementClass;
	UPROPERTY()
		TObjectPtr<UAnnouncement> Announcement;


	UPROPERTY(EditDefaultsOnly)
		float CrosshairSpreadMax = 16.f;

	void DrawCrosshair(UTexture2D* Texture, FVector2D Center, FVector2D Spread, FLinearColor Color);

};
