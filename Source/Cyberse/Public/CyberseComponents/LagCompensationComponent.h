// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

class ABlasterCharacter;
class AWeapon;

// used for server rewind. store the hitbox info
USTRUCT(BlueprintType)
struct FBoxInfo
{
	GENERATED_BODY()

	UPROPERTY()
		FVector Location;
	UPROPERTY()
		FRotator Rotation;
	UPROPERTY()
		FVector BoxExtent;
};

USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
		float Time;

	UPROPERTY()
		TMap<FName, FBoxInfo> HitBoxInfo;
};

USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
		bool bHitConfirmed;
	UPROPERTY()
		bool bHeadShot;
};

USTRUCT(BlueprintType)
struct FShotgunServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
		TMap<ABlasterCharacter*, uint32> HeadShots;
	UPROPERTY()
		TMap<ABlasterCharacter*, uint32> BodyShots;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CYBERSE_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULagCompensationComponent();
	friend class ABlasterCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void ShowFramePackage(const FFramePackage& Package, const FColor& Color);


	/**
	* Note: DamageCauser is to ensure that the damage is caused by the weapon that the player is holding by the time the shot is fired. 
	* However, this may be a possible cheating point if the param is hacked.
	* Alternatively, if using equipped weapon on the server, the weapon may be different (considering lag).
	* Maybe save the equipped weapon in the frame package.
	*/
	// For HitScan
	UFUNCTION(Server, Reliable)
	void ServerScoreRequest(
		ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize100& TraceStart,
		const FVector_NetQuantize100& HitLocation,
		float HitTime,
		AWeapon* DamageCauser
	);

	FServerSideRewindResult ServerSideRewind(
		class ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize100& TraceStart,
		const FVector_NetQuantize100& HitLocation,
		float HitTime);

	// For Projectile
	UFUNCTION(Server, Reliable)
		void ServerProjectileScoreRequest(
        ABlasterCharacter* HitCharacter,
        const FVector_NetQuantize100& TraceStart,
        const FVector_NetQuantize100& LaunchVelocity,
        float HitTime,
        AWeapon* DamageCauser
    );

	FServerSideRewindResult ProjectileServerSideRewind(
        class ABlasterCharacter* HitCharacter,
        const FVector_NetQuantize100& TraceStart,
        const FVector_NetQuantize100& LaunchVelocity,
        float HitTime);

	// For Shotgun
	UFUNCTION(Server, Reliable)
		void ServerShotgunScoreRequest(
        const TArray<ABlasterCharacter*>& HitCharacters,
        const FVector_NetQuantize100& TraceStart,
        const TArray<FVector_NetQuantize100>& HitLocations,
        float HitTime,
        AWeapon* DamageCauser
    );

	FShotgunServerSideRewindResult ShotgunServerSideRewind(
		const TArray<ABlasterCharacter*>& HitCharacters,
		const FVector_NetQuantize100& TraceStart,
		const TArray<FVector_NetQuantize100>& HitLocations,
		float HitTime);
protected:
	virtual void BeginPlay() override;

	FServerSideRewindResult ConfirmHit(
		const FFramePackage& Package,
		ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize100& TraceStart,
		const FVector_NetQuantize100& HitLocation);

	FServerSideRewindResult ProjectileConfirmHit(
        const FFramePackage& Package,
        ABlasterCharacter* HitCharacter,
        const FVector_NetQuantize100& TraceStart,
        const FVector_NetQuantize100& LaunchVelocity);


	FShotgunServerSideRewindResult ShotgunConfirmHit(
        const TArray<ABlasterCharacter*>& HitCharacters,
		const TArray<FFramePackage>& Packages,
        const FVector_NetQuantize100& TraceStart,
        const TArray<FVector_NetQuantize100>& HitLocations);


	// Utilities

	// linear interp between two frame package, A's time should be smaller than B's and Time should be between A and B
	FFramePackage LerpBetweenFrames(const FFramePackage& A, const FFramePackage& B, float Time);
    // get the frame package of a given character at the given time 
	FFramePackage GetFramePackage(ABlasterCharacter* Character, float Time);
	// cache the box positions of a given character's current frame (backup)
	void CacheBoxPositions(ABlasterCharacter* Character, FFramePackage& OutPackage);
	// move the boxes of a given character to the given location and toggle collisions
	void MoveBoxes(ABlasterCharacter* Character, const FFramePackage& Package, bool bCollisionEnabled);
	// save current frame's hitbox info
	void SaveFramePackage();
	// overload for updating a given frame package
	void SaveFramePackage(FFramePackage& Package);
	
	void DrawHitPoint(UWorld* World, FHitResult& ConfirmHitResult);

private:
	UPROPERTY()
		TObjectPtr<ABlasterCharacter> OwnerCharacter;
	UPROPERTY()
		TObjectPtr<class ABlasterPlayerController> OwnerController;


	TDoubleLinkedList<FFramePackage> FrameHistory;

	UPROPERTY(EditAnywhere)
		float MaxRecordTime = 4.f;

	UPROPERTY(EditAnywhere)
        bool bDebug = false;


};
