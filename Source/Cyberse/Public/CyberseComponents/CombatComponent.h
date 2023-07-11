// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HUD/CyberseHUD.h"
#include "Weapon/WeaponTypes.h"
#include "CharacterTypes/CombatState.h"
#include "CombatComponent.generated.h"


class AWeapon;
class ACyberseHUD;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CYBERSE_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	friend class ABlasterCharacter;

	UCombatComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SpawnDefaultWeapon();
	void EquipWeapon(AWeapon* Weapon);
	void SwapWeapon();
	void Reload();
	void ThrowGrenade();
	void PickupAmmo(EWeaponType WeaponType, int32 Amount);
	void DropAllWeapon();


	UFUNCTION(BlueprintCallable)
	void ShotgunShellReload();

	FORCEINLINE FVector GetHitTarget() const { return HitTarget; }
	FORCEINLINE int32 GetGrenadeCount() const { return GrenadeCount; }
	FORCEINLINE bool CanSwapWeapon() const { return EquippedWeapon != nullptr && SecondaryWeapon != nullptr && CombatState == ECombatState::ECS_Unoccupied; }
protected:
	virtual void BeginPlay() override;

	void SetAiming(bool bNewAiming);

	UFUNCTION(Server, Reliable)
		void ServerSetAiming(bool bNewAiming);

	void EquipPrimaryWeapon(AWeapon* WeaponToEquip);
	void EquipSecondaryWeapon(AWeapon* WeaponToEquip);

	UFUNCTION()
		void OnRep_EquippedWeapon();

	UFUNCTION()
		void OnRep_SecondaryWeapon();

	void FireButtonPressed(bool bPressed);

	UFUNCTION(Server, Reliable)
       void ServerReload();

	UFUNCTION(Server, Reliable)
		void ServerThrowGrenade();

	void HandleReload();

	void TraceUnderCrosshair(FHitResult& HitResult);

	void SetHUDCrosshairs(float DeltaTime);

	void AttachActorToSocket(AActor* Actor, FName SocketName);

private:
	UPROPERTY()
		TObjectPtr<class ABlasterCharacter> OwnerCharacter;
	UPROPERTY()
		TObjectPtr<class ABlasterPlayerController> Controller;
	UPROPERTY()
		TObjectPtr<ACyberseHUD> HUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
		TObjectPtr<AWeapon> EquippedWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon)
		TObjectPtr<AWeapon> SecondaryWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_IsAiming)
		bool bIsAiming;
	UPROPERTY(EditDefaultsOnly)
		float BaseWalkSpeed = 600.f;
	UPROPERTY(EditDefaultsOnly)
		float AimWalkSpeed = 450.f;

	bool bFireButtonPressed;

	bool bAimButtonPressed = false;

	UFUNCTION()
		void OnRep_IsAiming();

	/**
	* Crosshairs and HUD
	*/
	FVector HitTarget;
	float SpeedSpreadFactor;
	float AimingShrinkFactor;
	float JumpSpreadFactor;
	float ShootingSpreadFactor;

	UPROPERTY(EditDefaultsOnly)
		float CrosshairJumpSpread = 4.f;

	FHUDPackage HUDPackage;

	/**
	* Aiming and FOV
	*/
	// FOV when not aiming (base FOV)
	float DefaultFOV;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
		float ZoomedFOV = 30.f;

	float CurrentFOV;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
		float ZoomInterpSpeed = 20.f;

	void InterpFOV(float DeltaTime);

	/**
	* Fire
	*/
	FTimerHandle FireTimer;
	bool bCanFire = true;
	bool bLocallyReloading = false;
	
	bool IsLocalClient();
	bool CanFire();
	void StartFireTimer();
	void FireTimerFinished();
	void Fire();
	void FireProjectile();
	void FireHitScan();
	void FireShotgun();
	void LocalFire(const FVector_NetQuantize& TraceHitTarget);
	void LocalShotgunFire(const TArray<FVector_NetQuantize100>& TraceHitTargets);

	UFUNCTION(Server, Reliable)
		void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
		void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(Server, Reliable)
		void ServerShotgunFire(const TArray<FVector_NetQuantize100>& TraceHitTargets);

	UFUNCTION(NetMulticast, Reliable)
		void MulticastShotgunFire(const TArray<FVector_NetQuantize100>& TraceHitTargets);

	int32 AmountToReload();
	void UpdateAmmos();
	void UpdateShotgunAmmos();

	// Carried ammo for currently equipped weapon
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
		int32 CarriedAmmo;
	UPROPERTY(EditDefaultsOnly)
		TMap<EWeaponType, int32> CarriedAmmoMap;
	UPROPERTY(EditDefaultsOnly)
		TMap<EWeaponType, int32> MaxCarriedAmmoMap;


	UFUNCTION()
       void OnRep_CarriedAmmo();
	
	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
		ECombatState CombatState = ECombatState::ECS_Unoccupied;
	UFUNCTION()
		void OnRep_CombatState();

	/**
	* Grenade
	*/
	UPROPERTY(ReplicatedUsing = OnRep_GrenadeCount)
		int32 GrenadeCount = 2;

	UPROPERTY(EditDefaultsOnly)
		int32 MaxGrenadeCount = 4;

	UPROPERTY(EditDefaultsOnly)
        TSubclassOf<class AProjectile> GrenadeClass;

	UFUNCTION()
		void OnRep_GrenadeCount();

	UFUNCTION(Server, Reliable)
		void ServerSpawnGrenade(const FVector_NetQuantize& HitPoint);



	/**
	* Montage Notifies
	*/

	UFUNCTION(BlueprintCallable)
		void FinishReloading();

	UFUNCTION(BlueprintCallable)
		void FinishThrowingGrenade();

	UFUNCTION(BlueprintCallable)
		void FinishSwapping(); // end of swap animation

	UFUNCTION(BlueprintCallable)
		void FinishWeaponSwapped(); // the point where the weapon is actually swapped

	/**
	* Default Weapon
	*/
	UPROPERTY(EditDefaultsOnly)
        TSubclassOf<AWeapon> DefaultWeaponClass;
};
