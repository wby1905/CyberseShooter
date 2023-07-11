// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Components/TimelineComponent.h"

#include "CharacterTypes/TurningInPlace.h"
#include "CharacterTypes/CombatState.h"
#include "Interfaces/InteractWithCrosshairInterface.h"
#include "BlasterCharacter.generated.h"

class UInputAction;
class AWeapon;
class UCameraComponent;
class UAnimMontage;
class UBoxComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeftGame);

UCLASS()
class CYBERSE_API ABlasterCharacter : public ACharacter, public IInteractWithCrosshairInterface
{
	GENERATED_BODY()

public:
	ABlasterCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;
	virtual void Destroyed() override;
	virtual void OnRep_ReplicatedMovement() override;

	void SetOverlappingWeapon(AWeapon* Weapon);

	void Elim(const FString& Killer, bool bIsLeaving);

	/**
	* Leave Game (using Elim func)
	*/
	FOnLeftGame OnLeftGame;

	UFUNCTION(Server, Reliable)
		void ServerLeaveGame();

	
	/**
	* Montages
	*/
	void PlayFireMontage(bool bAiming);
	void PlayElimMontage();
	void PlayReloadMontage();
	void PlayShotgunEndMontage();
	void PlayThrowGrenadeMontage();
	void PlaySwapWeaponMontage();

	/**
	* HUDs
	*/
	void UpdateHUDHealth();
	void UpdateHUDShield();
	void UpdateHUDAmmo();

	UFUNCTION(Client, Reliable)
		void ClientSetCrosshairColor(FLinearColor Color) const;

	UFUNCTION(BlueprintImplementableEvent)
		void ShowSniperScopeWidget(bool bShow);

	UFUNCTION(NetMulticast, Reliable)
		void MulticastToggleTheLead(bool bEnabled);


	/**
	* Utils
	*/
	bool bFinishedSwapping = true;

	bool IsWeaponEquipped() const;
	bool IsAiming() const;
	bool IsLocallyReloading() const;
	TObjectPtr<AWeapon> GetEquippedWeapon() const;
	FVector GetHitTarget() const;
	float GetSpeed() const;
	ECombatState GetCombatState() const;
	FString GetWeaponName() const;
	void ToggleGrenadeAttachment(bool bEnabled);
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FORCEINLINE TObjectPtr<UCameraComponent> GetCamera() const { return Camera; }
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	FORCEINLINE bool IsElimmed() const { return bElimmed; }
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE void SetHealth(float NewHealth) { Health = NewHealth; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	FORCEINLINE float GetShield() const { return Shield; }
	FORCEINLINE void SetShield(float NewShield) { Shield = NewShield; }
	FORCEINLINE float GetMaxShield() const { return MaxShield; }
	FORCEINLINE TObjectPtr<UStaticMeshComponent> GetGrenadeMesh() const { return GrenadeMesh; }
	FORCEINLINE void ToggleDisableGameplay(bool bEnabled) { bDisableGameplay = bEnabled; OnRep_DisableGameplay(); }
	FORCEINLINE TObjectPtr<class UCombatComponent> GetCombatComponent() const { return CombatComponent; }
	FORCEINLINE TObjectPtr<class UBuffComponent> GetBuffComponent() const { return BuffComponent; }
	FORCEINLINE TObjectPtr<class ULagCompensationComponent> GetLagCompensationComponent() const { return LagCompensation; }
	FORCEINLINE TObjectPtr<AWeapon> GetOverlappingWeapon() const { return OverlappingWeapon; }
	FORCEINLINE TMap<FName, TObjectPtr<UBoxComponent>> GetHitBoxes() const { return HitBoxes; }
protected:
	virtual void BeginPlay() override;


private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
		TObjectPtr<class USpringArmComponent> CameraBoom;
	UPROPERTY(VisibleAnywhere, Category = Camera)
		TObjectPtr<UCameraComponent> Camera;
	UPROPERTY(EditAnywhere)
		TObjectPtr<class UWidgetComponent> OverheadWidget;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UCombatComponent> CombatComponent;
	UPROPERTY(VisibleAnywhere)
		TObjectPtr<UBuffComponent> BuffComponent;
	UPROPERTY(VisibleAnywhere)
		TObjectPtr<ULagCompensationComponent> LagCompensation;
	UPROPERTY()
		TObjectPtr<class ABlasterPlayerController> BlasterPlayerController;
	UPROPERTY()
		TObjectPtr<class ABlasterPlayerState> BlasterPlayerState;

	UPROPERTY(Replicated)
		TObjectPtr<AWeapon> OverlappingWeapon;

	/**
	* Animation Properties
	*/
	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;
	ETurningInPlace TurningInPlace;

	bool bRotateRootBone;
	float TurnThreshold = 5.f;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	void AimOffset(float DeltaTime);
	void SimProxiesTurn();
	void TurnInPlace(float DeltaTime);

	/**
	* Inputs
	*/
	UPROPERTY(ReplicatedUsing = OnRep_DisableGameplay)
		bool bDisableGameplay = false;

	UPROPERTY(EditAnywhere, Category = Input)
		TObjectPtr<class UInputMappingContext> InputMappingContext;
	UPROPERTY(EditAnywhere, Category = Input)
		TObjectPtr<UInputAction> MoveAction;
	UPROPERTY(EditAnywhere, Category = Input)
		TObjectPtr<UInputAction> LookAction;
	UPROPERTY(EditAnywhere, Category = Input)
		TObjectPtr<UInputAction> JumpAction;
	UPROPERTY(EditAnywhere, Category = Input)
		TObjectPtr<UInputAction> FireAction;
	UPROPERTY(EditAnywhere, Category = Input)
		TObjectPtr<UInputAction> CrouchAction;
	UPROPERTY(EditAnywhere, Category = Input)
		TObjectPtr<UInputAction> EKeyAction;
	UPROPERTY(EditAnywhere, Category = Input)
		TObjectPtr<UInputAction> AimAction;
	UPROPERTY(EditAnywhere, Category = Input)
		TObjectPtr<UInputAction> ReloadAction;
	UPROPERTY(EditAnywhere, Category = Input)
		TObjectPtr<UInputAction> ThrowGrenadeAction;
	UPROPERTY(EditAnywhere, Category = Input)
		TObjectPtr<UInputAction> SwitchWeaponAction;
	UPROPERTY(EditAnywhere, Category = Input)
		TObjectPtr<UInputAction> MenuAction;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void CrouchButton();
	void AimButtonPressed();
	void AimButtonReleased();
	void FireButtonPressed();
	void FireButtonReleased();
	void ReloadButton();
	void ThrowGrenadeButton();
	void EKeyPressed();
	void SwitchWeaponButton();
	void MenuButton();

	UFUNCTION(Server, Reliable)
		void ServerEKeyPressed();
	UFUNCTION(Server, Reliable)
		void ServerSwitchWeapon();
	UFUNCTION()
		void OnRep_DisableGameplay();

	// Menu
	bool bMenuOpen = false;
	TObjectPtr<class UReturnToMainMenu> ReturnMenuWidget;

	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<UReturnToMainMenu> ReturnMenuWidgetClass;
	
	void OpenMenu();
	void CloseMenu();

	/**
	* Hide camera when close to wall
	*/
	UPROPERTY(EditAnywhere, Category = Camera)
		float CameraThreshold = 200.f;
	void HideCameraWhenClose();

	/**
	* Montages
	*/
	UPROPERTY(EditDefaultsOnly, Category = "Combat | Montages")
        TObjectPtr<UAnimMontage> FireWeaponMontage;
	UPROPERTY(EditDefaultsOnly, Category = "Combat | Montages")
		TObjectPtr<UAnimMontage> HitReactMontage;
	UPROPERTY(EditDefaultsOnly, Category = "Combat | Montages")
		TObjectPtr<UAnimMontage> ElimMontage;
	UPROPERTY(EditDefaultsOnly, Category = "Combat | Montages")
		TObjectPtr<UAnimMontage> ReloadMontage;
	UPROPERTY(EditDefaultsOnly, Category = "Combat | Montages")
		TObjectPtr<UAnimMontage> GrenadeMontage;
	UPROPERTY(EditDefaultsOnly, Category = "Combat | Montages")
		TObjectPtr<UAnimMontage> SwapWeaponMontage;


	void PlayHitReactMontage();
	void PlayMontage(UAnimMontage* MontageToPlay, float InPlayRate = 1.f, FName StartSectionName = NAME_None);

	/**
	* Player Health and Elimination
	*/
	UPROPERTY(EditDefaultsOnly, Category = "Player Stats")
		float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
		float Health = 100.f;


	bool bElimmed = false;
	FTimerHandle ElimTimerHandle;
	float RespawnTime;

	UPROPERTY(EditDefaultsOnly, Category = Elim)
		float ElimDelay = 7.f;

	void ElimTimerFinished();

	UFUNCTION()
		void OnRep_Health(float OldHealth);

	UFUNCTION(NetMulticast, Reliable)
		void MulticastElim(const FString& Killer, bool bIsLeaving);

	/**
	* Leave Game (using Elim func)
	*/
	bool bLeftGame = false;

	/**
	* Player Shield
	*/
	UPROPERTY(EditDefaultsOnly, Category = "Player Stats")
        float MaxShield = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Shield, EditDefaultsOnly, Category = "Player Stats")
		float Shield = 0.f;

	UFUNCTION()
		void OnRep_Shield(float OldShield);

	/**
	* Dissolve Effect
	*/
	FOnTimelineFloat DissolveTrack;

	UPROPERTY(VisibleAnywhere, Category = Elim)
		TObjectPtr<UTimelineComponent> DissolveTimeline;
	UPROPERTY(EditAnywhere, Category = Elim)
		TObjectPtr<UCurveFloat> DissolveCurve;
	// Dynamic instance that changes at runtime
	UPROPERTY(VisibleAnywhere, Category = Elim)
		TObjectPtr<UMaterialInstanceDynamic> DynamicDissolveMI;
	// Static instance that is set in the editor, used as a base for the dynamic instance
	UPROPERTY(EditAnywhere, Category = Elim)
		TObjectPtr<UMaterialInstance> DissolveMaterialInstance;

	UFUNCTION()
		void UpdateDissolveMaterial(float DissolveValue);
	void StartDissolve();

	/**
	* Elimmed Sounds & Effects 
	*/
	UPROPERTY(EditDefaultsOnly, Category = Elim)
        TObjectPtr<USoundBase> ElimSound;
    UPROPERTY(EditDefaultsOnly, Category = Elim)
		TObjectPtr<UParticleSystem> ElimParticle;
	UPROPERTY(VisibleAnywhere, Category = Elim)
		TObjectPtr<UParticleSystemComponent> ElimParticleComponent;
	UPROPERTY(EditDefaultsOnly, Category = Elim)
        TObjectPtr<class UNiagaraSystem> CrownSystem;
	UPROPERTY(VisibleAnywhere, Category = Elim)
		TObjectPtr<class UNiagaraComponent> CrownComponent;

	/**
	*  Show Damage Amount
	*/
	UPROPERTY(VisibleAnywhere, Category = Combat)
        TObjectPtr<class UDamageUI> DamageUI;
	UPROPERTY(EditDefaultsOnly, Category = Combat)
        TObjectPtr<UWidgetComponent> DamageUIComponent;

	void ShowDamageAmount(float DamageAmount, FVector HitLocation, bool bIsHeadshot, bool bIsShieldDamage);
	UFUNCTION(Client, Reliable)
		void ClientShowDamageAmount(float DamageAmount, FVector HitLocation, bool bIsHeadshot, bool bIsShieldDamage, ABlasterCharacter* HittedCharacter);

	/**
	* Grenade
	*/

	UPROPERTY(VisibleAnywhere)
		TObjectPtr<UStaticMeshComponent> GrenadeMesh;


	// Poll for any relevant classes and initialize HUD
	void PollInit();

	/**
	* Hit Boxes used for server-side rewind
	* The name is the same as the bone name
	*/
	UPROPERTY()
		TMap<FName, TObjectPtr<UBoxComponent>> HitBoxes;

	UPROPERTY(EditAnywhere, Category = "Hit Boxes")
		TObjectPtr<UBoxComponent> Head;
	UPROPERTY(EditAnywhere, Category = "Hit Boxes")
		TObjectPtr<UBoxComponent> Pelvis;
	UPROPERTY(EditAnywhere, Category = "Hit Boxes")
		TObjectPtr<UBoxComponent> Spine_02;
	UPROPERTY(EditAnywhere, Category = "Hit Boxes")
		TObjectPtr<UBoxComponent> Spine_03;
	UPROPERTY(EditAnywhere, Category = "Hit Boxes")
		TObjectPtr<UBoxComponent> Lower_Arm_L;
	UPROPERTY(EditAnywhere, Category = "Hit Boxes")
		TObjectPtr<UBoxComponent> Lower_Arm_R;
	UPROPERTY(EditAnywhere, Category = "Hit Boxes")
		TObjectPtr<UBoxComponent> Upper_Arm_L;
	UPROPERTY(EditAnywhere, Category = "Hit Boxes")
		TObjectPtr<UBoxComponent> Upper_Arm_R;
	UPROPERTY(EditAnywhere, Category = "Hit Boxes")
		TObjectPtr<UBoxComponent> Hand_L;
	UPROPERTY(EditAnywhere, Category = "Hit Boxes")
		TObjectPtr<UBoxComponent> Hand_R;
	UPROPERTY(EditAnywhere, Category = "Hit Boxes")
		TObjectPtr<UBoxComponent> Backpack;
	UPROPERTY(EditAnywhere, Category = "Hit Boxes")
		TObjectPtr<UBoxComponent> Blanket;
	UPROPERTY(EditAnywhere, Category = "Hit Boxes")
		TObjectPtr<UBoxComponent> Thigh_L;
	UPROPERTY(EditAnywhere, Category = "Hit Boxes")
		TObjectPtr<UBoxComponent> Thigh_R;
	UPROPERTY(EditAnywhere, Category = "Hit Boxes")
		TObjectPtr<UBoxComponent> Calf_L;
	UPROPERTY(EditAnywhere, Category = "Hit Boxes")
		TObjectPtr<UBoxComponent> Calf_R;
	UPROPERTY(EditAnywhere, Category = "Hit Boxes")
		TObjectPtr<UBoxComponent> Foot_L;
	UPROPERTY(EditAnywhere, Category = "Hit Boxes")
		TObjectPtr<UBoxComponent> Foot_R;

	// Not Used: This might be a Bug (See in Constructor)
	void InitHitBoxes();
	void InitHitBox(UBoxComponent* BoxComponent, FName BoneName, float DamageMultiplier);
	
};
