// Fill out your copyright notice in the Description page of Project Settings.


#include "CyberseComponents/CombatComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"

#include "Weapon/Weapon.h"
#include "Weapon/Projectile.h"
#include "Weapon/Shotgun.h"
#include "Character/BlasterCharacter.h"
#include "Controller/BlasterPlayerController.h"
#include "GameMode/CyberseGameMode.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (OwnerCharacter && OwnerCharacter->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshair(HitResult);
		HitTarget = HitResult.ImpactPoint;

		SetHUDCrosshairs(DeltaTime);
		InterpFOV(DeltaTime);
	}

}


void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, SecondaryWeapon);
	DOREPLIFETIME(UCombatComponent, bIsAiming);
	DOREPLIFETIME(UCombatComponent, CombatState);

	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UCombatComponent, GrenadeCount, COND_OwnerOnly);
}

void UCombatComponent::SpawnDefaultWeapon()
{
	UWorld* World = GetWorld();
	ACyberseGameMode* GameMode = Cast<ACyberseGameMode>(UGameplayStatics::GetGameMode(World));
	if (GameMode && World && OwnerCharacter && !OwnerCharacter->IsElimmed() && DefaultWeaponClass)
	{
		AWeapon* StartingWeapon = World->SpawnActor<AWeapon>(DefaultWeaponClass);
		if (StartingWeapon)
		{
            EquipWeapon(StartingWeapon);
			StartingWeapon->SetIsDefaultWeapon(true);
			StartingWeapon->TogglePickupWidget(false);
			StartingWeapon->SetActorEnableCollision(false);
			StartingWeapon->SetOwner(OwnerCharacter);
        }
	}
}

void UCombatComponent::EquipWeapon(AWeapon* Weapon)
{
	if (Weapon == nullptr || CombatState != ECombatState::ECS_Unoccupied)
        return;
	if (EquippedWeapon == nullptr || SecondaryWeapon != nullptr)
	{
		// no primary weapon or has both weapons
		EquipPrimaryWeapon(Weapon);
	}
	else if (EquippedWeapon)
	{
		if (EquippedWeapon->IsDefaultWeapon())
			EquipPrimaryWeapon(Weapon);
		else
			EquipSecondaryWeapon(Weapon);
	}
}

void UCombatComponent::SwapWeapon()
{
	if (OwnerCharacter == nullptr || !CanSwapWeapon() || !OwnerCharacter->HasAuthority())
        return;
	CombatState = ECombatState::ECS_Swapping;
	OwnerCharacter->PlaySwapWeaponMontage();
	OwnerCharacter->bFinishedSwapping = false;
}

void UCombatComponent::EquipPrimaryWeapon(AWeapon* WeaponToEquip)
{
	if (OwnerCharacter == nullptr || WeaponToEquip == nullptr) return;
	if (EquippedWeapon)
	{
		EquippedWeapon->Dropped();
	}

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetOwner(OwnerCharacter);
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	EquippedWeapon->OnRep_Owner();
	OnRep_CarriedAmmo();
	OnRep_EquippedWeapon();
	if (EquippedWeapon->IsEmpty())
	{
		Reload();
	}
}

void UCombatComponent::EquipSecondaryWeapon(AWeapon* WeaponToEquip)
{
	if (OwnerCharacter == nullptr || WeaponToEquip == nullptr) return;
	SecondaryWeapon = WeaponToEquip;
	SecondaryWeapon->SetOwner(OwnerCharacter);
	OnRep_SecondaryWeapon();
}

void UCombatComponent::Reload()
{
	if (CarriedAmmo > 0 && CombatState == ECombatState::ECS_Unoccupied && EquippedWeapon && !bLocallyReloading)
	{
		if (!EquippedWeapon->IsFull())
        {
            ServerReload();
			HandleReload();
			bLocallyReloading = true;
        }
	}
}

void UCombatComponent::ThrowGrenade()
{
	if (CombatState != ECombatState::ECS_Unoccupied || EquippedWeapon == nullptr || GrenadeCount <= 0)
        return;
	CombatState = ECombatState::ECS_Throwing;
	if (OwnerCharacter)
	{
		OwnerCharacter->PlayThrowGrenadeMontage();
		AttachActorToSocket(EquippedWeapon, FName("LeftHandSocket"));
		if (!OwnerCharacter->HasAuthority())
		{
			ServerThrowGrenade();
		}
	}
}

void UCombatComponent::PickupAmmo(EWeaponType WeaponType, int32 Amount)
{
	if (CarriedAmmoMap.Contains(WeaponType) && MaxCarriedAmmoMap.Contains(WeaponType))
	{
        CarriedAmmoMap[WeaponType] = FMath::Clamp(CarriedAmmoMap[WeaponType] + Amount, 0, MaxCarriedAmmoMap[WeaponType]);

		if (EquippedWeapon && CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
		{
			CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		}
		OnRep_CarriedAmmo();

    }

	if (EquippedWeapon && EquippedWeapon->IsEmpty() && EquippedWeapon->GetWeaponType() == WeaponType)
	{
        Reload();
    }

}

void UCombatComponent::DropAllWeapon()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Dropped();
	}
	if (SecondaryWeapon)
	{
		SecondaryWeapon->Dropped();
	}

}

void UCombatComponent::ShotgunShellReload()
{
	if (OwnerCharacter && OwnerCharacter->HasAuthority())
		UpdateShotgunAmmos();
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (OwnerCharacter)
	{
		OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;

		if (OwnerCharacter->GetCamera())
		{
			DefaultFOV = OwnerCharacter->GetCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}
	}
	SpawnDefaultWeapon();
	HUDPackage.CrosshairColor = FLinearColor::White;
}

void UCombatComponent::SetAiming(bool bNewAiming)
{
	if (OwnerCharacter == nullptr || EquippedWeapon == nullptr || CombatState != ECombatState::ECS_Unoccupied) return;
	// let client immediately set the aiming state
	bIsAiming = bNewAiming;
    OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = bNewAiming ? AimWalkSpeed : BaseWalkSpeed;
	ServerSetAiming(bNewAiming);
	if (OwnerCharacter->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
	{
		OwnerCharacter->ShowSniperScopeWidget(bNewAiming);
	}
	if (OwnerCharacter->IsLocallyControlled())
	{
		// the actural isaiming value on client.
		bAimButtonPressed = bNewAiming;
	}
}

void UCombatComponent::ServerSetAiming_Implementation(bool bNewAiming)
{
	bIsAiming = bNewAiming;
	if (OwnerCharacter)
	{
		OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = bNewAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && OwnerCharacter)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		EquippedWeapon->SetHUDAmmo();
		AttachActorToSocket(EquippedWeapon, FName("RightHandSocket"));

		OwnerCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
		OwnerCharacter->bUseControllerRotationYaw = true;

		if (EquippedWeapon->EquipSound)
		{
			UGameplayStatics::PlaySound2D(this, EquippedWeapon->EquipSound);
		}

		if (OwnerCharacter->IsLocallyControlled() && Controller)
		{
			Controller->SetHUDWeaponName(OwnerCharacter->GetWeaponName());
		}
	}
	
}

void UCombatComponent::OnRep_SecondaryWeapon()
{
	if (SecondaryWeapon && OwnerCharacter)
	{
		// not the primary weapon
		if (SecondaryWeapon->EquipSound && SecondaryWeapon->GetWeaponState() != EWeaponState::EWS_Equipped)
		{
            UGameplayStatics::PlaySound2D(this, SecondaryWeapon->EquipSound);
        }
		SecondaryWeapon->SetWeaponState(EWeaponState::EWS_Secondary);
		AttachActorToSocket(SecondaryWeapon, FName("BackpackSocket"));

	}
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;

	if (bFireButtonPressed)
	{
		Fire();
    }
}

void UCombatComponent::ServerReload_Implementation()
{
	if (OwnerCharacter == nullptr) return;
	CombatState = ECombatState::ECS_Reloading;
	if (!OwnerCharacter->IsLocallyControlled())
		HandleReload();
}

void UCombatComponent::ServerThrowGrenade_Implementation()
{
    CombatState = ECombatState::ECS_Throwing;
	if (OwnerCharacter)
	{
		OwnerCharacter->PlayThrowGrenadeMontage();
		AttachActorToSocket(EquippedWeapon, FName("LeftHandSocket"));
	}
}

void UCombatComponent::HandleReload()
{
	if (OwnerCharacter == nullptr) return;
	OwnerCharacter->PlayReloadMontage();
	if (EquippedWeapon && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle && OwnerCharacter && bIsAiming)
	{
		SetAiming(false);
		OwnerCharacter->ShowSniperScopeWidget(false);
	}
}

void UCombatComponent::TraceUnderCrosshair(FHitResult& HitResult)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
        GEngine->GameViewport->GetViewportSize(ViewportSize);
    }

	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	if (UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0), 
		CrosshairLocation, 
		CrosshairWorldPosition, 
		CrosshairWorldDirection)
		)
	{
        FVector TraceStart = CrosshairWorldPosition;

		// add offset to trace start to avoid hitting the actors behind the character
		if (OwnerCharacter)
		{
			float DistanceToCharacter = (OwnerCharacter->GetActorLocation() - TraceStart).Size();
			TraceStart += CrosshairWorldDirection * (DistanceToCharacter + 50.f);
		}


        FVector TraceEnd = CrosshairWorldPosition + CrosshairWorldDirection * TRACE_LENGTH;
		GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility);

		if (!HitResult.bBlockingHit)
		{
			HitResult.ImpactPoint = TraceEnd;
		}

    }

}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(OwnerCharacter->GetController()) : Controller;
	if (Controller)
	{
		HUD = HUD == nullptr ? Cast<ACyberseHUD>(Controller->GetHUD()) : HUD;
	}
	if (HUD)
	{
		if (EquippedWeapon)
		{
			HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
			HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
			HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
			HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
			HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
		}
		else
		{
			HUDPackage.CrosshairsCenter = nullptr;
			HUDPackage.CrosshairsBottom = nullptr;
			HUDPackage.CrosshairsLeft = nullptr;
			HUDPackage.CrosshairsRight = nullptr;
			HUDPackage.CrosshairsTop = nullptr;
			return;
		}

		if (HUDPackage.CrosshairColor != FLinearColor::White)
		{
			HUDPackage.CrosshairColor = FLinearColor::LerpUsingHSV(HUDPackage.CrosshairColor, FLinearColor::White, DeltaTime);
		}
		// Change spread based on Speed, fallind, aiming and crouching
		FVector2D WalkSpeedRange(0.f, BaseWalkSpeed);
		FVector Velocity = OwnerCharacter->GetVelocity();
		Velocity.Z = 0.f;
		SpeedSpreadFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, FVector2D(0.5f, 1.5f), Velocity.Size());
		if (OwnerCharacter->GetCharacterMovement()->IsCrouching())
		{
            SpeedSpreadFactor *= 0.5f;
        }

		if (bIsAiming)
		{
			AimingShrinkFactor = FMath::FInterpTo(AimingShrinkFactor, 1.f, DeltaTime, 30.f);
		}
		else
		{
			AimingShrinkFactor = FMath::FInterpTo(AimingShrinkFactor, 0.f, DeltaTime, 30.f);

		}

		if (OwnerCharacter->GetCharacterMovement()->IsFalling())
		{
			JumpSpreadFactor = FMath::FInterpTo(JumpSpreadFactor, CrosshairJumpSpread, DeltaTime, 30.f);
		}
		else
		{
			JumpSpreadFactor = FMath::FInterpTo(JumpSpreadFactor, 0.f, DeltaTime, 30.f);

		}

		ShootingSpreadFactor = FMath::FInterpTo(ShootingSpreadFactor, 0.f, DeltaTime, 5.f);

		HUDPackage.CrosshairSpreadMultiplier =
			0.5f - AimingShrinkFactor + 
			(SpeedSpreadFactor + JumpSpreadFactor + ShootingSpreadFactor) * EquippedWeapon->GetBasicSpreadFactor();

		HUD->SetHUDPackage(HUDPackage);
	}

}

void UCombatComponent::AttachActorToSocket(AActor* Actor, FName SocketName)
{
	if (Actor == nullptr || OwnerCharacter == nullptr || OwnerCharacter->GetMesh() == nullptr) return;
	const USkeletalMeshSocket* Socket = OwnerCharacter->GetMesh()->GetSocketByName(SocketName);
	if (Socket)
	{
        Socket->AttachActor(Actor, OwnerCharacter->GetMesh());
    }
}

void UCombatComponent::OnRep_IsAiming()
{
	if (OwnerCharacter && OwnerCharacter->IsLocallyControlled())
	{
		// fix local client's isaiming
		bIsAiming = bAimButtonPressed;
	}
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr) return;

	if (bIsAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
	}

	if (OwnerCharacter->GetCamera())
	{
		OwnerCharacter->GetCamera()->SetFieldOfView(CurrentFOV);
	}
}

bool UCombatComponent::IsLocalClient()
{
	return OwnerCharacter && !OwnerCharacter->HasAuthority() && OwnerCharacter->IsLocallyControlled();
}

bool UCombatComponent::CanFire()
{
	if (EquippedWeapon == nullptr || EquippedWeapon->IsEmpty()) return false;
	// shotgun can fire while reloading
	if (bCanFire && CombatState == ECombatState::ECS_Reloading && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun) return true;
	if (bLocallyReloading) return false;
	return bCanFire && CombatState == ECombatState::ECS_Unoccupied;
}

void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || OwnerCharacter == nullptr) return;
	OwnerCharacter->GetWorldTimerManager().SetTimer(
		FireTimer,
		this,
		&UCombatComponent::FireTimerFinished,
		EquippedWeapon->GetFireDelay());
}

void UCombatComponent::FireTimerFinished()
{
	if (EquippedWeapon == nullptr) return;
	bCanFire = true;
	if (bFireButtonPressed && EquippedWeapon->IsAutomatic())
	{
		Fire();
	}

	if (EquippedWeapon->IsEmpty())
    {
        Reload();
    }
}

void UCombatComponent::Fire()
{
	if (!CanFire()) return;
	if (EquippedWeapon == nullptr) return;
	if (EquippedWeapon->IsEmpty())
	{
		Reload();
		return;
	}
	bCanFire = false;
	ShootingSpreadFactor += 0.5f;
	ShootingSpreadFactor = FMath::Clamp(ShootingSpreadFactor, 0.f, 2.f) * EquippedWeapon->GetBasicSpreadFactor();

	StartFireTimer();

	switch (EquippedWeapon->GetFireType())
	{
		case EFireType::EFT_Projectile:
            FireProjectile();
            break;
		case EFireType::EFT_HitScan:
			FireHitScan();
            break;
		// Shotgun is handled in FireShotgun() (special case)
		case EFireType::EFT_Shotgun:
			FireShotgun();
            return;
	}
	// Show animation on local client in advance
	if (IsLocalClient())
	{
		LocalFire(HitTarget);
	}
	ServerFire(HitTarget);
}

void UCombatComponent::LocalFire(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr) return;
	if (OwnerCharacter && CombatState == ECombatState::ECS_Unoccupied)
	{
		OwnerCharacter->PlayFireMontage(bIsAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UCombatComponent::LocalShotgunFire(const TArray<FVector_NetQuantize100>& TraceHitTargets)
{
	if (OwnerCharacter && (CombatState == ECombatState::ECS_Unoccupied || CombatState == ECombatState::ECS_Reloading))
	{
		if (AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon))
		{
			OwnerCharacter->PlayFireMontage(bIsAiming);
			Shotgun->FireShotgun(TraceHitTargets);
			// allow shotgun to fire at once when reloading and stop reloading then.
			CombatState = ECombatState::ECS_Unoccupied;
			bLocallyReloading = false;
		}
	}
}

void UCombatComponent::FireProjectile()
{
	if (EquippedWeapon && EquippedWeapon->GetFireType() == EFireType::EFT_Projectile)
	{
		// calculate scatter and sync the trace end among all clients
		HitTarget = EquippedWeapon->IsScatterEnabled() ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
	}
}

void UCombatComponent::FireHitScan()
{
	if (EquippedWeapon && EquippedWeapon->GetFireType() == EFireType::EFT_HitScan)
	{
		// calculate scatter and sync the trace end among all clients
		HitTarget = EquippedWeapon->IsScatterEnabled() ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
	}
}

void UCombatComponent::FireShotgun()
{
	if (AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon))
	{
		TArray<FVector_NetQuantize100> TraceEnds;
		Shotgun->ShotgunTraceEndWithScatter(HitTarget, TraceEnds);
		if (IsLocalClient())
		{
			LocalShotgunFire(TraceEnds);
		}
		ServerShotgunFire(TraceEnds);
    }
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (IsLocalClient()) return;
	LocalFire(TraceHitTarget);
}

void UCombatComponent::ServerShotgunFire_Implementation(const TArray<FVector_NetQuantize100>& TraceHitTargets)
{
	MulticastShotgunFire(TraceHitTargets);
}
void UCombatComponent::MulticastShotgunFire_Implementation(const TArray<FVector_NetQuantize100>& TraceHitTargets)
{
	if (IsLocalClient()) return;
	LocalShotgunFire(TraceHitTargets);
}

void UCombatComponent::OnRep_CarriedAmmo()
{
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(OwnerCharacter->GetController()) : Controller;
	if (Controller)
	{
		Controller->SetHUDTotalAmmo(CarriedAmmo);
	}
	bool bJumpToShotgunEnd =
		CombatState == ECombatState::ECS_Reloading &&
		EquippedWeapon &&
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun &&
		CarriedAmmo == 0;
	if (bJumpToShotgunEnd && OwnerCharacter)
	{
		OwnerCharacter->PlayShotgunEndMontage();
	}
}

int32 UCombatComponent::AmountToReload()
{
if (EquippedWeapon == nullptr) return 0;
    int32 ReloadAmount = EquippedWeapon->GetMaxAmmo() - EquippedWeapon->GetCurrentAmmo();
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
    {
        ReloadAmount = FMath::Min(ReloadAmount, CarriedAmmoMap[EquippedWeapon->GetWeaponType()]);
		return FMath::Max(ReloadAmount, 0);
	}
	return 0;
}

void UCombatComponent::UpdateAmmos()
{
	int32 ReloadAmount = AmountToReload();
	if (EquippedWeapon && CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		OnRep_CarriedAmmo();
	}
	EquippedWeapon->AddAmmo(ReloadAmount);
}

void UCombatComponent::UpdateShotgunAmmos()
{
	if (EquippedWeapon == nullptr) return;
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= 1;
        CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
        OnRep_CarriedAmmo();
    }
	EquippedWeapon->AddAmmo(1);
	bCanFire = true;
	if (EquippedWeapon->IsFull() || CarriedAmmo == 0)
	{
		// Jump to Shotgun End section
		if (OwnerCharacter) OwnerCharacter->PlayShotgunEndMontage();
	}

}

void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Unoccupied:
		if (bFireButtonPressed)
		{
			Fire();
		}
		break;
	case ECombatState::ECS_Reloading:
		if (OwnerCharacter && !OwnerCharacter->IsLocallyControlled()) HandleReload();
		break;
	case ECombatState::ECS_Throwing:
		if (OwnerCharacter && !OwnerCharacter->IsLocallyControlled())
		{
			OwnerCharacter->PlayThrowGrenadeMontage();
			AttachActorToSocket(EquippedWeapon, FName("LeftHandSocket"));
		}
		break;
	case ECombatState::ECS_Swapping:
		if (OwnerCharacter && !OwnerCharacter->IsLocallyControlled())
		{
            OwnerCharacter->PlaySwapWeaponMontage();
        }
        break;
	}
}

void UCombatComponent::OnRep_GrenadeCount()
{
	if (Controller)
	{
        Controller->SetHUDGrenadeAmount(GrenadeCount);
    }
}

void UCombatComponent::ServerSpawnGrenade_Implementation(const FVector_NetQuantize& HitPoint)
{
	if (GrenadeClass && OwnerCharacter && OwnerCharacter->GetGrenadeMesh())
	{
		const FVector SpawnLocation = OwnerCharacter->GetGrenadeMesh()->GetComponentLocation();
		FVector ToTarget = HitPoint - SpawnLocation;
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = OwnerCharacter;
		SpawnParameters.Instigator = OwnerCharacter;

		if (UWorld* World = GetWorld())
		{
			AProjectile* Grenade = World->SpawnActor<AProjectile>(GrenadeClass, SpawnLocation, ToTarget.Rotation(), SpawnParameters);
			if (Grenade)
			{
				UStaticMeshComponent* GrenadeMesh = Grenade->GetProjectileMesh();

			}
		}

		GrenadeCount = FMath::Max(GrenadeCount - 1, 0);
		OnRep_GrenadeCount();
	}
}

void UCombatComponent::FinishReloading()
{
	if (OwnerCharacter && OwnerCharacter->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmos();
	}
	if (bFireButtonPressed)
	{
		Fire();
	}
	bLocallyReloading = false;
}

void UCombatComponent::FinishThrowingGrenade()
{
	CombatState = ECombatState::ECS_Unoccupied;
	AttachActorToSocket(EquippedWeapon, FName("RightHandSocket"));
	if (OwnerCharacter)
	{
		OwnerCharacter->ToggleGrenadeAttachment(false);
		if (OwnerCharacter->IsLocallyControlled())
		{
			ServerSpawnGrenade(HitTarget);
		}
	}

	if (bFireButtonPressed)
	{
        Fire();
    }

}

void UCombatComponent::FinishSwapping()
{

	// Issue: https://forums.unrealengine.com/t/animnotify-inconsistently-doesnt-trigger-on-the-server/429809/4
	// AnimNotify doesn't trigger on the server sometimes (especially too laggy), and set notify tick type to "branching point" helps (but not always)
	// This seems an unressolved ue bug. Maybe add more notifies for redundancy.
	if (OwnerCharacter)
	{
		OwnerCharacter->bFinishedSwapping = true;
		if (OwnerCharacter->HasAuthority())
		{
			CombatState = ECombatState::ECS_Unoccupied;
			OnRep_CombatState();
		}
    }
}

void UCombatComponent::FinishWeaponSwapped()
{
	if (OwnerCharacter && OwnerCharacter->HasAuthority())
	{
		AWeapon* TempWeapon = SecondaryWeapon;
		EquipSecondaryWeapon(EquippedWeapon);
		EquippedWeapon = nullptr;
		EquipPrimaryWeapon(TempWeapon);
	}

}
