// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BlasterCharacter.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/BoxComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Sound/SoundBase.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/DamageEvents.h"

#include "../Cyberse.h"
#include "HUD/OverheadWidget.h"
#include "HUD/CyberseHUD.h"
#include "HUD/ReturnToMainMenu.h"
#include "HUD/DamageUI.h"
#include "Weapon/Weapon.h"
#include "Weapon/WeaponTypes.h"
#include "CyberseComponents/CombatComponent.h"
#include "CyberseComponents/BuffComponent.h"
#include "CyberseComponents/LagCompensationComponent.h"
#include "Character/BlasterAnimInstance.h"
#include "Controller/BlasterPlayerController.h"
#include "GameMode/CyberseGameMode.h"
#include "State/CyberseGameState.h"
#include "State/BlasterPlayerState.h"


ABlasterCharacter::ABlasterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	// Camera
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->TargetArmLength = 500.0f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	// Collisions
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

	// Movement
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

    GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCharacterMovement()->RotationRate.Yaw = 850.f;


	// Components init
	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	CombatComponent->SetIsReplicated(true);

	BuffComponent = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	BuffComponent->SetIsReplicated(true);

	LagCompensation = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensation"));
	LagCompensation->SetIsReplicated(true);

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	DamageUIComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("DamageUIComponent"));
	DamageUIComponent->SetupAttachment(RootComponent);

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimeline"));

	/**
	* Init Grenade Mesh
	*/

	GrenadeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GrenadeMesh"));
	GrenadeMesh->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
	GrenadeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GrenadeMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	GrenadeMesh->MarkRenderStateDirty();
	GrenadeMesh->SetRenderCustomDepth(true);

	/**
	* Init Hit Boxes
	* (used for Server-side rewind only)
	*/
	// there is bugs in UE editor that if using member function to init hit boxes, the editor will not show details panel.
	// so need to init manually
	//InitHitBoxes();
	FName head = FName(TEXT("head"));
	Head = CreateDefaultSubobject<UBoxComponent>(head);
	Head->SetupAttachment(GetMesh(), head);
	HitBoxes.Add(head, Head);

	FName pelvis = FName(TEXT("pelvis"));
	Pelvis = CreateDefaultSubobject<UBoxComponent>(pelvis);
	Pelvis->SetupAttachment(GetMesh(), pelvis);
	HitBoxes.Add(pelvis, Pelvis);

	FName spine_02 = FName(TEXT("spine_02"));
	Spine_02 = CreateDefaultSubobject<UBoxComponent>(spine_02);
	Spine_02->SetupAttachment(GetMesh(), spine_02);
	HitBoxes.Add(spine_02, Spine_02);

	FName spine_03 = FName(TEXT("spine_03"));
	Spine_03 = CreateDefaultSubobject<UBoxComponent>(spine_03);
	Spine_03->SetupAttachment(GetMesh(), spine_03);
	HitBoxes.Add(spine_03, Spine_03);

	FName upperarm_l = FName(TEXT("upperarm_l"));
	Upper_Arm_L = CreateDefaultSubobject<UBoxComponent>(upperarm_l);
	Upper_Arm_L->SetupAttachment(GetMesh(), upperarm_l);
	HitBoxes.Add(upperarm_l, Upper_Arm_L);

	FName upperarm_r = FName(TEXT("upperarm_r"));
	Upper_Arm_R = CreateDefaultSubobject<UBoxComponent>(upperarm_r);
	Upper_Arm_R->SetupAttachment(GetMesh(), upperarm_r);
	HitBoxes.Add(upperarm_r, Upper_Arm_R);

	FName lowerarm_l = FName(TEXT("lowerarm_l"));
	Lower_Arm_L = CreateDefaultSubobject<UBoxComponent>(lowerarm_l);
	Lower_Arm_L->SetupAttachment(GetMesh(), lowerarm_l);
	HitBoxes.Add(lowerarm_l, Lower_Arm_L);

	FName lowerarm_r = FName(TEXT("lowerarm_r"));
	Lower_Arm_R = CreateDefaultSubobject<UBoxComponent>(lowerarm_r);
	Lower_Arm_R->SetupAttachment(GetMesh(), lowerarm_r);
	HitBoxes.Add(lowerarm_r, Lower_Arm_R);

	FName hand_l = FName(TEXT("hand_l"));
	Hand_L = CreateDefaultSubobject<UBoxComponent>(hand_l);
	Hand_L->SetupAttachment(GetMesh(), hand_l);
	HitBoxes.Add(hand_l, Hand_L);

	FName hand_r = FName(TEXT("hand_r"));
	Hand_R = CreateDefaultSubobject<UBoxComponent>(hand_r);
	Hand_R->SetupAttachment(GetMesh(), hand_r);
	HitBoxes.Add(hand_r, Hand_R);

	FName thigh_l = FName(TEXT("thigh_l"));
	Thigh_L = CreateDefaultSubobject<UBoxComponent>(thigh_l);
	Thigh_L->SetupAttachment(GetMesh(), thigh_l);
	HitBoxes.Add(thigh_l, Thigh_L);

	FName thigh_r = FName(TEXT("thigh_r"));
	Thigh_R = CreateDefaultSubobject<UBoxComponent>(thigh_r);
	Thigh_R->SetupAttachment(GetMesh(), thigh_r);
	HitBoxes.Add(thigh_r, Thigh_R);

	FName calf_l = FName(TEXT("calf_l"));
	Calf_L = CreateDefaultSubobject<UBoxComponent>(calf_l);
	Calf_L->SetupAttachment(GetMesh(), calf_l);
	HitBoxes.Add(calf_l, Calf_L);

	FName calf_r = FName(TEXT("calf_r"));
	Calf_R = CreateDefaultSubobject<UBoxComponent>(calf_r);
	Calf_R->SetupAttachment(GetMesh(), calf_r);
	HitBoxes.Add(calf_r, Calf_R);

	FName foot_l = FName(TEXT("foot_l"));
	Foot_L = CreateDefaultSubobject<UBoxComponent>(foot_l);
	Foot_L->SetupAttachment(GetMesh(), foot_l);
	HitBoxes.Add(foot_l, Foot_L);

	FName foot_r = FName(TEXT("foot_r"));
	Foot_R = CreateDefaultSubobject<UBoxComponent>(foot_r);
	Foot_R->SetupAttachment(GetMesh(), foot_r);
	HitBoxes.Add(foot_r, Foot_R);

	FName backpack = FName(TEXT("backpack"));
	Backpack = CreateDefaultSubobject<UBoxComponent>(backpack);
	Backpack->SetupAttachment(GetMesh(), backpack);
	HitBoxes.Add(backpack, Backpack);

	FName blanket = FName(TEXT("blanket_l"));
	Blanket = CreateDefaultSubobject<UBoxComponent>(blanket);
	Blanket->SetupAttachment(GetMesh(), blanket);
	HitBoxes.Add(blanket, Blanket);
	// Again, there is a bug that I cannot adjust the box component in the details panel, so I have to set it here
	Blanket->SetBoxExtent(FVector(31.414990, 16.397247, 21.190906));
	Blanket->AddLocalOffset(FVector(-0.846522, -0.064059, 0));
	Blanket->AddLocalRotation(FRotator(0, 4.327495, 0));


	for (auto Box : HitBoxes)
	{
		if (Box.Value)
		{
			Box.Value->SetCollisionObjectType(ECC_HitBox);
			Box.Value->SetCollisionResponseToAllChannels(ECR_Ignore);
			Box.Value->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block);
			Box.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	PollInit();

	// ----------------------------------------------------------
	/**
	* Note: turn in place is not so effective in multiplayer, the commented code is one solution, but it cannot sync the animation
	* So I just leave it for now.
	*/
	//if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	//{
	AimOffset(DeltaTime);
	//}
	//else
	//{
	//	TimeSinceLastMovementReplication += DeltaTime;
	//	if (TimeSinceLastMovementReplication >= 0.25f)
	//	{
	//		OnRep_ReplicatedMovement();
 //       }
	//}
	// ----------------------------------------------------------


	HideCameraWhenClose();

	// map pitch to -90 to 90
	AO_Pitch = GetBaseAimRotation().Pitch > 180.f ? GetBaseAimRotation().Pitch - 360.f : GetBaseAimRotation().Pitch;

	if (IsElimmed() && IsLocallyControlled())
    {
		RespawnTime = FMath::Max(0.f, RespawnTime - DeltaTime);
		if (BlasterPlayerController)
		{
			BlasterPlayerController->SetHUDRespawnTimer(RespawnTime);
		}
	} 
}

void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		// Crouch
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ABlasterCharacter::CrouchButton);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &ABlasterCharacter::CrouchButton);
		// Aim
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &ABlasterCharacter::AimButtonPressed);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ABlasterCharacter::AimButtonReleased);
		// Fire
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ABlasterCharacter::FireButtonPressed);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ABlasterCharacter::FireButtonReleased);
		// E Key
		EnhancedInputComponent->BindAction(EKeyAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::EKeyPressed);
		// Reload
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::ReloadButton);
		// Grenade
		EnhancedInputComponent->BindAction(ThrowGrenadeAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::ThrowGrenadeButton);
		// Switch Weapon
		EnhancedInputComponent->BindAction(SwitchWeaponAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::SwitchWeaponButton);
		// Menu
		EnhancedInputComponent->BindAction(MenuAction, ETriggerEvent::Triggered, this, &ABlasterCharacter::MenuButton);
    }

}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterCharacter, Health);
	DOREPLIFETIME(ABlasterCharacter, Shield);
	
	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ABlasterCharacter, bDisableGameplay, COND_OwnerOnly);
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (CombatComponent)
	{
		CombatComponent->OwnerCharacter = this;
		UpdateHUDAmmo();
	}
	if (BuffComponent)
	{
        BuffComponent->OwnerCharacter = this;
    }
	if (LagCompensation)
	{
		LagCompensation->OwnerCharacter = this;
	}
}

float ABlasterCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// Only server can handle damage event
	if (!HasAuthority() || bElimmed) return 0.f;
	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	float OldShield = Shield;

	bool bIsShieldDamage = false;

	// TODO: Currently the damageevent is not being used(Only default), so cannot get the hitinfo of these two.
	bool bIsHeadShot = false; 
	FVector HitLocation = FVector::ZeroVector;


	if (DamageAmount <= Shield)
	{
		Shield = FMath::Clamp(Shield - DamageAmount, 0.f, MaxShield);
		OnRep_Shield(OldShield);
		bIsShieldDamage = true;
	}
	else
	{
        Shield = 0.f;
        OnRep_Shield(OldShield);

		float OldHealth = Health;
		Health = FMath::Clamp(Health + OldShield - DamageAmount, 0.f, MaxHealth);
		OnRep_Health(OldHealth);
	}


	if (Health == 0.f)
	{
		ACyberseGameMode* CyberseGameMode = GetWorld()->GetAuthGameMode<ACyberseGameMode>();
		if (CyberseGameMode && BlasterPlayerController)
		{
			ABlasterPlayerController* InstigatorPlayerController = Cast<ABlasterPlayerController>(EventInstigator);
			CyberseGameMode->PlayerEliminated(this, BlasterPlayerController, InstigatorPlayerController);
		}

	}
	if (DamageCauser)
	{
		if (ABlasterCharacter* Hitter= Cast<ABlasterCharacter>(DamageCauser->GetOwner()))
		{
			Hitter->ClientSetCrosshairColor(FColor::Red);
			Hitter->ClientShowDamageAmount(DamageAmount, HitLocation, bIsHeadShot, bIsShieldDamage, this);
		}
	}

	return DamageAmount;
}

void ABlasterCharacter::Destroyed()
{
	if (ElimParticleComponent)
	{
		ElimParticleComponent->DestroyComponent();
	}
	if (BlasterPlayerController)
	{
		BlasterPlayerController->ToggleHUDBlur(false);
	}
	Super::Destroyed();
}

void ABlasterCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
    SimProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}

void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	AWeapon* LastWeapon = OverlappingWeapon;
	OverlappingWeapon = Weapon;
	if (IsLocallyControlled())
	{
		if (LastWeapon)
		{
			LastWeapon->TogglePickupWidget(false);
		}
		if (OverlappingWeapon && !OverlappingWeapon->IsDefaultWeapon())
		{
			OverlappingWeapon->TogglePickupWidget(true);
		}
	}
}

void ABlasterCharacter::Elim(const FString& Killer, bool bIsLeaving)
{
	if (IsWeaponEquipped())
	{
		CombatComponent->DropAllWeapon();		
	}
	MulticastElim(Killer, bIsLeaving);
}

void ABlasterCharacter::PlayFireMontage(bool bAiming)
{
	if (!IsWeaponEquipped()) return;
	FName SectionName;
	SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
	PlayMontage(FireWeaponMontage, 1.f, SectionName);
}

void ABlasterCharacter::PlayElimMontage()
{
	PlayMontage(ElimMontage, 1.f);
}

void ABlasterCharacter::PlayReloadMontage()
{
	if (!IsWeaponEquipped()) return;
	FName SectionName;
	switch (CombatComponent->EquippedWeapon->GetWeaponType())
	{
	case EWeaponType::EWT_Rifle:
		SectionName = FName("Rifle");
		break;
	case EWeaponType::EWT_RocketLauncher:
		SectionName = FName("RocketLauncher");
		break;
	case EWeaponType::EWT_Pistol:
		SectionName = FName("Pistol");
		break;
	case EWeaponType::EWT_SMG:
		SectionName = FName("SMG");
        break;
	case EWeaponType::EWT_Shotgun:
		SectionName = FName("Shotgun");
		break;
	case EWeaponType::EWT_SniperRifle:
		SectionName = FName("SniperRifle");
        break;
	case EWeaponType::EWT_GrenadeLauncher:
		SectionName = FName("GrenadeLauncher");
        break;
	default:
		SectionName = FName("Rifle");
	}
	PlayMontage(ReloadMontage, 1.f, SectionName);

}

void ABlasterCharacter::PlayShotgunEndMontage()
{
	if (!IsWeaponEquipped()) return;
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->Montage_JumpToSection(FName("ShotgunEnd"), ReloadMontage);
	}
}

void ABlasterCharacter::PlayThrowGrenadeMontage()
{
	PlayMontage(GrenadeMontage, 1.f);
	ToggleGrenadeAttachment(true);
}

void ABlasterCharacter::PlaySwapWeaponMontage()
{
	PlayMontage(SwapWeaponMontage, 1.f);
}

void ABlasterCharacter::UpdateHUDHealth()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? GetController<ABlasterPlayerController>() : BlasterPlayerController;
	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void ABlasterCharacter::UpdateHUDShield()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? GetController<ABlasterPlayerController>() : BlasterPlayerController;
	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDShield(Shield, MaxShield);
	}
}

void ABlasterCharacter::UpdateHUDAmmo()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? GetController<ABlasterPlayerController>() : BlasterPlayerController;
	if (BlasterPlayerController && IsWeaponEquipped())
	{
		BlasterPlayerController->SetHUDTotalAmmo(CombatComponent->CarriedAmmo);
		BlasterPlayerController->SetHUDWeaponAmmo(CombatComponent->EquippedWeapon->GetCurrentAmmo());
		BlasterPlayerController->SetHUDWeaponName(GetWeaponName());
	}
}

void ABlasterCharacter::ClientSetCrosshairColor_Implementation(FLinearColor Color) const
{
	if (CombatComponent)
	{
		CombatComponent->HUDPackage.CrosshairColor = Color;
	}
}

void ABlasterCharacter::MulticastToggleTheLead_Implementation(bool bEnabled)
{
	if (CrownSystem == nullptr) return;
	if (CrownComponent == nullptr)
	{
		CrownComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			CrownSystem,
			GetCapsuleComponent(),
			FName(),
			GetActorLocation() + FVector(0.f, 0.f, 120.f),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false);
	}
	if (CrownComponent)
	{
		if (bEnabled)
		{
            CrownComponent->Activate();
        }
		else
		{
            CrownComponent->Deactivate();
        }
	}
}

bool ABlasterCharacter::IsWeaponEquipped() const
{
	return CombatComponent && CombatComponent->EquippedWeapon;
}

bool ABlasterCharacter::IsAiming() const
{
	return CombatComponent && CombatComponent->bIsAiming;
}

bool ABlasterCharacter::IsLocallyReloading() const
{
	return CombatComponent && CombatComponent->bLocallyReloading;
}

TObjectPtr<AWeapon> ABlasterCharacter::GetEquippedWeapon() const
{
	if (CombatComponent)
        return CombatComponent->EquippedWeapon;
	return nullptr;
}

FVector ABlasterCharacter::GetHitTarget() const
{
	if (CombatComponent)
		return CombatComponent->GetHitTarget();
	return FVector();
}

float ABlasterCharacter::GetSpeed() const
{
	FVector Velocity{ GetVelocity() };
	Velocity.Z = 0;
	return Velocity.Size();
}

ECombatState ABlasterCharacter::GetCombatState() const
{
	if (CombatComponent == nullptr) return ECombatState::ECS_MAX;
	return CombatComponent->CombatState;
}

FString ABlasterCharacter::GetWeaponName() const
{
	if (IsWeaponEquipped())
	{
		TEnumAsByte<EWeaponType> WeaponType = CombatComponent->EquippedWeapon->GetWeaponType();
		FText WeaponName = UEnum::GetDisplayValueAsText(WeaponType);
		return WeaponName.ToString();
	}
	return FString(TEXT("None"));
}

void ABlasterCharacter::ToggleGrenadeAttachment(bool bEnabled)
{
	if (GrenadeMesh)
	{
        GrenadeMesh->SetVisibility(bEnabled);
    }
}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (IsLocallyControlled())
	{
		APlayerController* PlayerController = Cast<APlayerController>(GetController());
		if (UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (InputMappingContext)
			{
				EnhancedInputSubsystem->AddMappingContext(InputMappingContext, 0);
			}
		}
	}
	ToggleGrenadeAttachment(false);

}

void ABlasterCharacter::AimOffset(float DeltaTime)
{
	if (!IsWeaponEquipped()) return;
	float Speed = GetSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir) // standing still and not jumping
	{
		bRotateRootBone = true;
		FRotator CurrentRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentRotation, StartingAimRotation);
		AO_Yaw = DeltaRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}

		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}
	else
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		bUseControllerRotationYaw = true;
		AO_Yaw = 0.f;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

}

void ABlasterCharacter::SimProxiesTurn()
{
	if (!IsWeaponEquipped()) return;
	bRotateRootBone = false;
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

	float Speed = GetSpeed();
	if (Speed > 0.f) return;

	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetBaseAimRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;

	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if (ProxyYaw < -TurnThreshold)
		{
            TurningInPlace = ETurningInPlace::ETIP_Left;
        }
	}

}

void ABlasterCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)
    {
        TurningInPlace = ETurningInPlace::ETIP_Left;
    }
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

// ---------------------------------------------------------
// Input Actions
// ---------------------------------------------------------
void ABlasterCharacter::Move(const FInputActionValue& Value)
{
	FVector2D Direction = Value.Get<FVector2D>();
	FRotator Rotation = GetControlRotation();
	FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);
	FVector ControllerForward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(ControllerForward, Direction.Y);
	FVector ControllerRight = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(ControllerRight, Direction.X);
}

void ABlasterCharacter::Look(const FInputActionValue& Value)
{
	FVector2D Direction = Value.Get<FVector2D>();
    AddControllerYawInput(Direction.X);
	AddControllerPitchInput(Direction.Y);
}

void ABlasterCharacter::CrouchButton()
{
	if (bIsCrouched)
	{
        UnCrouch();
    }
	else
	{
        Crouch();
    }
}

void ABlasterCharacter::AimButtonPressed()
{
	if (CombatComponent)
	{
		CombatComponent->SetAiming(true);
	}
}

void ABlasterCharacter::AimButtonReleased()
{
	if (CombatComponent)
	{
		CombatComponent->SetAiming(false);
    }
}

void ABlasterCharacter::FireButtonPressed()
{
	if (bDisableGameplay) return;
	if (CombatComponent)
	{
		CombatComponent->FireButtonPressed(true);
    }
}

void ABlasterCharacter::FireButtonReleased()
{
	if (CombatComponent)
	{
        CombatComponent->FireButtonPressed(false);
    }
}

void ABlasterCharacter::ReloadButton()
{
	if (bDisableGameplay) return;
	if (CombatComponent)
    {
        CombatComponent->Reload();
    }
}

void ABlasterCharacter::ThrowGrenadeButton()
{
	if (bDisableGameplay) return;
	if (CombatComponent)
	{
        CombatComponent->ThrowGrenade();
    }
}

void ABlasterCharacter::EKeyPressed()
{
    if (bDisableGameplay) return;
	ServerEKeyPressed();
	SetOverlappingWeapon(nullptr);
}

void ABlasterCharacter::SwitchWeaponButton()
{
	if (bDisableGameplay) return;

	if (CombatComponent && CombatComponent->CanSwapWeapon())
	{
		ServerSwitchWeapon();
		if (!HasAuthority())
		{
			PlaySwapWeaponMontage();
			CombatComponent->CombatState = ECombatState::ECS_Swapping;
			bFinishedSwapping = false;
		}
	}
}

void ABlasterCharacter::MenuButton()
{
	if (bMenuOpen)
	{
		CloseMenu();
	}
	else
	{
		OpenMenu();
	}
}

void ABlasterCharacter::ServerEKeyPressed_Implementation()
{
	if (bDisableGameplay) return;
	if (CombatComponent)
	{
		CombatComponent->EquipWeapon(OverlappingWeapon);
	}
}

void ABlasterCharacter::ServerSwitchWeapon_Implementation()
{
	if (bDisableGameplay) return;
	if (CombatComponent)
	{
        CombatComponent->SwapWeapon();
    }
}

void ABlasterCharacter::OnRep_DisableGameplay()
{
	if (bDisableGameplay)
	{
		FireButtonReleased();
	}
}

void ABlasterCharacter::OpenMenu()
{
	if (bMenuOpen) return;
	if (ReturnMenuWidget == nullptr)
	{
		ReturnMenuWidget = CreateWidget<UReturnToMainMenu>(GetWorld(), ReturnMenuWidgetClass);
	}
	if (ReturnMenuWidget)
	{
		ReturnMenuWidget->MenuSetup();
		bMenuOpen = true;
    }
}

void ABlasterCharacter::CloseMenu()
{
	if (!bMenuOpen) return;
	if (ReturnMenuWidget)
	{
        ReturnMenuWidget->MenuTeardown();
        bMenuOpen = false;
    }
}

// ---------------------------------------------------------

void ABlasterCharacter::HideCameraWhenClose()
{
	if (!IsLocallyControlled()) return;
	if (FVector::Dist(GetActorLocation(), Camera->GetComponentLocation()) < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (CombatComponent && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponMesh())
		{
			CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
    }
	else
	{
		GetMesh()->SetVisibility(true);
		if (CombatComponent && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponMesh())
		{
			CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
    }
}


void ABlasterCharacter::PlayHitReactMontage()
{
	if (!IsWeaponEquipped()) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	FName SectionName("FromBack");

	PlayMontage(HitReactMontage, 1.f, SectionName);
}

void ABlasterCharacter::PlayMontage(UAnimMontage* MontageToPlay, float InPlayRate, FName StartSectionName)
{
    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

    if (!AnimInstance || !MontageToPlay) return;
    AnimInstance->Montage_Play(MontageToPlay, InPlayRate, EMontagePlayReturnType::MontageLength, 0.f);
	if (StartSectionName != NAME_None)
	{
        AnimInstance->Montage_JumpToSection(StartSectionName, MontageToPlay);
    }
}

void ABlasterCharacter::ElimTimerFinished()
{
	ACyberseGameMode* CyberseGameMode = GetWorld()->GetAuthGameMode<ACyberseGameMode>();
	if (CyberseGameMode && !bLeftGame)
	{
		CyberseGameMode->RequestRespawn(this, Controller);
    }
	if (bLeftGame && IsLocallyControlled())
	{
		OnLeftGame.Broadcast();
	}
}

void ABlasterCharacter::OnRep_Health(float OldHealth)
{
	UpdateHUDHealth();
	if (OldHealth > Health)
	{
		PlayHitReactMontage();
	}
}

void ABlasterCharacter::OnRep_Shield(float OldShield)
{
	UpdateHUDShield();
	if (OldShield > Shield)
	{
        PlayHitReactMontage();
    }
}

void ABlasterCharacter::MulticastElim_Implementation(const FString& Killer, bool bIsLeaving)
{
	bLeftGame = bIsLeaving;
	bElimmed = true;
	PlayElimMontage();

	// Start Dissolve Effect
	if (DissolveMaterialInstance)
	{
		DynamicDissolveMI = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMI);
		DynamicDissolveMI->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMI->SetScalarParameterValue(TEXT("Glow"), 200.f);

		StartDissolve();
	}

	// Disable Movement
	GetCharacterMovement()->DisableMovement();
    GetCharacterMovement()->StopMovementImmediately();
	FireButtonReleased();
	if (BlasterPlayerController)
    {
		DisableInput(BlasterPlayerController);
    }
	// Disable collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Spawn Elim VFX and SFX
	FVector SpawnLocation = GetActorLocation();
	if (ElimParticle)
	{
		SpawnLocation.Z += 200.f;
		ElimParticleComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ElimParticle, SpawnLocation);
	}
	if (ElimSound)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), ElimSound, SpawnLocation);
    }
	if (CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}


	// Enable Respawn HUD
	if (BlasterPlayerController && IsLocallyControlled() && !bIsLeaving)
	{
		RespawnTime = ElimDelay;
		BlasterPlayerController->ToggleHUDBlur(true);
		BlasterPlayerController->SetHUDKilledBy(Killer);
		BlasterPlayerController->SetHUDWeaponAmmo(0);
		BlasterPlayerController->SetHUDWeaponName(TEXT("None"));
		if (CombatComponent)
		{
            BlasterPlayerController->SetHUDGrenadeAmount(CombatComponent->GetGrenadeCount());
        }

	}

	if (IsLocallyControlled() && IsWeaponEquipped() && CombatComponent->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
	{
		ShowSniperScopeWidget(false);
	}

	if (GetLocalRole() > ROLE_SimulatedProxy)
	{
		GetWorldTimerManager().SetTimer(
			ElimTimerHandle,
			this,
			&ABlasterCharacter::ElimTimerFinished,
			ElimDelay
		);
	}
}

void ABlasterCharacter::ServerLeaveGame_Implementation()
{
	ACyberseGameMode* CyberseGameMode = GetWorld()->GetAuthGameMode<ACyberseGameMode>();
	if (CyberseGameMode)
	{
		if (IsLocallyControlled())
		{
			// maybe overrided, so just leave it here
			CyberseGameMode->ReturnToMainMenuHost(); // the game is terminated, no need to update game state
		}
		else
		{
			CyberseGameMode->PlayerLeftGame(BlasterPlayerState);
		}
	}
	else if (AGameModeBase* GameMode = GetWorld()->GetAuthGameMode()) // other game modes, like the lobby
	{
		if (IsLocallyControlled())
		{
			GameMode->ReturnToMainMenuHost();
		}
		else
		{
			Elim(FString("Left Game"), true);
		}

	}
}

void ABlasterCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if (DynamicDissolveMI)
	{
        DynamicDissolveMI->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
    }
}

void ABlasterCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &ABlasterCharacter::UpdateDissolveMaterial);
	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->SetLooping(false);
		DissolveTimeline->Play();
	}
}

void ABlasterCharacter::ShowDamageAmount(float DamageAmount, FVector HitLocation, bool bIsHeadshot, bool bIsShieldDamage)
{
	UE_LOG(LogTemp, Warning, TEXT("ShowDamageAmount"));
	if (DamageUIComponent)
	{
		DamageUI = DamageUI == nullptr ? Cast<UDamageUI>(DamageUIComponent->GetWidget()) : DamageUI;
		DamageUIComponent->SetWorldLocation(HitLocation);
	}
	if (DamageUI)
	{
		// TODO: Headshot is not implemented yet
		FLinearColor DamageColor = bIsHeadshot ? FLinearColor::Red : FLinearColor(
			1.0f,
			FMath::RandRange(0.f, 0.2f),
			FMath::RandRange(0.f, 0.2f)
		);
		if (bIsShieldDamage)
        {
			DamageColor = FLinearColor(
				FMath::RandRange(0.f, 0.2f),
				FMath::RandRange(0.f, 0.2f),
				1.0f
			);
        }
		DamageUI->SetDamageText(DamageAmount, DamageColor);
	}
}

void ABlasterCharacter::ClientShowDamageAmount_Implementation(float DamageAmount, FVector HitLocation, bool bIsHeadshot, bool bIsShieldDamage, ABlasterCharacter* HittedCharacter)
{
	if (HittedCharacter == nullptr) return;
	if (HitLocation.IsZero())
	{
		//TODO: temporary solution, because not implement damage event yet
		HitLocation = CombatComponent->GetHitTarget(); 
		FVector HitDirection = (HitLocation - GetActorLocation()).GetSafeNormal2D();
		HitLocation = HitLocation - HitDirection * 10.f;
		HitLocation.Z += 50.f;
		// jitter the location
		HitLocation.X += FMath::RandRange(-10.f, 10.f);
		HitLocation.Y += FMath::RandRange(-10.f, 10.f);
	}
	HittedCharacter->ShowDamageAmount(DamageAmount, HitLocation, bIsHeadshot, bIsShieldDamage);
}

void ABlasterCharacter::PollInit()
{
	if (BlasterPlayerState == nullptr)
    {
        BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
		if (BlasterPlayerState)
		{
			// refresh
			BlasterPlayerState->AddToScore(0.f);
			BlasterPlayerState->AddToDeathCount(0);
			ACyberseGameState* GameState = GetWorld()->GetGameState<ACyberseGameState>();
			if (GameState && GameState->GetTopScoringPlayers().Contains(BlasterPlayerState))
			{
				MulticastToggleTheLead(true);
			}
		}

    }


	if (BlasterPlayerController == nullptr)
	{
		BlasterPlayerController = GetController<ABlasterPlayerController>();
		if (BlasterPlayerController && IsLocallyControlled())
		{
			if (UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(BlasterPlayerController->GetLocalPlayer()))
			{
				if (InputMappingContext)
				{
					EnhancedInputSubsystem->AddMappingContext(InputMappingContext, 0);
				}
			}
		
			UpdateHUDHealth();
			UpdateHUDShield();
			UpdateHUDAmmo();
			BlasterPlayerController->ToggleHUDBlur(false);
			if (CombatComponent)
				BlasterPlayerController->SetHUDGrenadeAmount(CombatComponent->GetGrenadeCount());
		}

	}
}

void ABlasterCharacter::InitHitBoxes()
{
	InitHitBox(Head, FName("head"), 1.f);
	InitHitBox(Pelvis, FName("pelvis"), 1.f);
	InitHitBox(Spine_02, FName("spine_02"), 1.f);
	InitHitBox(Spine_03, FName("spine_03"), 1.f);
	InitHitBox(Upper_Arm_L, FName("upperarm_l"), 1.f);
	InitHitBox(Upper_Arm_R, FName("upperarm_r"), 1.f);
	InitHitBox(Lower_Arm_L, FName("lowerarm_l"), 1.f);
	InitHitBox(Lower_Arm_R, FName("lowerarm_r"), 1.f);
	InitHitBox(Hand_L, FName("hand_l"), 1.f);
	InitHitBox(Hand_R, FName("hand_r"), 1.f);
	InitHitBox(Thigh_L, FName("thigh_l"), 1.f);
	InitHitBox(Thigh_R, FName("thigh_r"), 1.f);
	InitHitBox(Calf_L, FName("calf_l"), 1.f);
	InitHitBox(Calf_R, FName("calf_r"), 1.f);
	InitHitBox(Foot_L, FName("foot_l"), 1.f);
	InitHitBox(Foot_R, FName("foot_r"), 1.f);
	InitHitBox(Backpack, FName("backpack"), 1.f);
	InitHitBox(Blanket, FName("blanket_l"), 1.f);
}

// Only called in constructor
void ABlasterCharacter::InitHitBox(UBoxComponent* BoxComponent, FName BoneName, float DamageMultiplier)
{
	// TODO: apply damage multiplier
	if (BoxComponent) return;
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(BoneName);
	BoxComponent->SetupAttachment(GetMesh(), BoneName);
	BoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}



