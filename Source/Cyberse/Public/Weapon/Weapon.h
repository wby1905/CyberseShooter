// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon/WeaponTypes.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Secondary UMETA(DisplayName = "Secondary"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	EWS_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class CYBERSE_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Owner() override;


	virtual void Fire(const FVector& HitTarget);
	void Dropped();

	void TogglePickupWidget(bool bVisible);
	void ToggleCollision(bool bEnabled);
	void SetWeaponState(EWeaponState State);
	void SetHUDAmmo();
	void AddAmmo(int32 AmmoToAdd);

	FVector_NetQuantize TraceEndWithScatter(const FVector& HitTarget) const;


/**
* Textures for the crosshairs
*/
	UPROPERTY(EditDefaultsOnly, Category = "Weapon Properties")
		TObjectPtr<class UTexture2D> CrosshairsCenter;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon Properties")
		TObjectPtr<class UTexture2D> CrosshairsLeft;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon Properties")
		TObjectPtr<class UTexture2D> CrosshairsRight;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon Properties")
		TObjectPtr<class UTexture2D> CrosshairsBottom;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon Properties")
		TObjectPtr<class UTexture2D> CrosshairsTop;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon Properties")
		TObjectPtr<class USoundBase> EquipSound;




	FORCEINLINE TObjectPtr<USkeletalMeshComponent> GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
	FORCEINLINE bool IsAutomatic() const { return bIsAutomatic; }
	FORCEINLINE float GetFireDelay() const { return FireDelay; }
	FORCEINLINE bool IsEmpty() const { return CurrentAmmo <= 0; }
	FORCEINLINE bool IsFull() const { return CurrentAmmo == MaxAmmo; }
	FORCEINLINE int32 GetCurrentAmmo() const { return CurrentAmmo; }
	FORCEINLINE int32 GetMaxAmmo() const { return MaxAmmo; }
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE EFireType GetFireType() const { return FireType; }
	FORCEINLINE float GetBasicSpreadFactor() const { return BasicSpreadFactor; }
	FORCEINLINE bool IsDefaultWeapon() const { return bIsDefaultWeapon; }
	FORCEINLINE void SetIsDefaultWeapon(bool bDefault) { bIsDefaultWeapon = bDefault; }
	FORCEINLINE EWeaponState GetWeaponState() const { return WeaponState; }
	FORCEINLINE bool IsScatterEnabled() const { return bUseScatter; }
	FORCEINLINE float GetDamage() const { return Damage; }
	FORCEINLINE float GetHeadshotMultiplier() const { return HeadshotMultiplier; }
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
		virtual void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY()
		TObjectPtr<class ABlasterPlayerController> PlayerController;
	
	UPROPERTY()
		TObjectPtr<class ABlasterCharacter> PlayerCharacter;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon Properties")
		float Damage = 10.f;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon Properties")
		float HeadshotMultiplier = 2.f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon Properties")
		EWeaponType WeaponType;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon Properties")
		EFireType FireType = EFireType::EFT_Projectile;


	/**
	* Params for scatter
	*/
	UPROPERTY(EditDefaultsOnly, Category = "Weapon Properties")
		float DistanceToSphere = 800.f;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon Properties")
		float SphereRadius = 75.f;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon Properties")
		bool bUseScatter = false;


	/**
	* Server Side Rewind
	*/
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
		bool bDefaultUseServerSideRewind = false;
	UPROPERTY(Replicated)
		bool bUseServerSideRewind = false; // may be changed according to the ping (disabled if ping is too high)
	UFUNCTION()
		void OnHighPing(bool bHighPing);

private:
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
		TObjectPtr<USkeletalMeshComponent> WeaponMesh;
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
		TObjectPtr<class USphereComponent> SphereComponent;
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
		TObjectPtr<class UWidgetComponent> PickupWidget;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon Properties")
		TObjectPtr<class UAnimationAsset> FireAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon Properties")
        TSubclassOf<class ABulletCasing> CasingClass;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
		bool bIsAutomatic = true;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
		float FireDelay = .15f;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
		float BasicSpreadFactor = 1.f;

	bool bIsDefaultWeapon = false;


	/**
	* Ammo
	*/
	UPROPERTY(EditDefaultsOnly, Category = "Weapon Properties")
        int32 MaxAmmo = 30;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon Properties")
		int32 CurrentAmmo = 30;
	// the number of unprocessed server requests for SpendRound
	// Incremented in SpendRound, Decremented in ClientUpdateAmmo
	int32 Sequence = 0;

	void SpendRound();

	UFUNCTION(Client, Reliable)
		void ClientUpdateAmmo(int32 ServerAmmo);
	UFUNCTION(NetMulticast, Reliable)
		void MultiCastAddAmmo(int32 AmmoToAdd);
	/**
	* Zoomed FOV
	*/
	UPROPERTY(EditDefaultsOnly, Category = "Weapon Properties")
		float ZoomedFOV = 30.f;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon Properties")
		float ZoomInterpSpeed = 20.f;

	/**
	* Weapon State
	*/
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties", ReplicatedUsing = OnRep_WeaponState)
		EWeaponState WeaponState;

	UFUNCTION()
	void OnRep_WeaponState();

	void HandleEquipped();
	void HandleDropped();
	void HandleSecondary();


	/**
	*	Custom Depth
	*/
	void ToggleCustomDepth(bool bEnabled);

	// recoil
	UPROPERTY(EditDefaultsOnly, Category = "Weapon Properties")
        float RecoilPitchMin = 0.f;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon Properties")
		float RecoilPitchMax = 0.f;


};
