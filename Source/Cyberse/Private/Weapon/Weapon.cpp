// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimationAsset.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/KismetMathLibrary.h"

#include "Character/BlasterCharacter.h"
#include "Weapon/BulletCasing.h"
#include "Controller/BlasterPlayerController.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_PURPLE);
	WeaponMesh->MarkRenderStateDirty();
	ToggleCustomDepth(true);

	// only generate overlap events on server
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->SetupAttachment(RootComponent);
	SphereComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState);
	DOREPLIFETIME_CONDITION(AWeapon, bUseServerSideRewind, COND_OwnerOnly);
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();
	if (GetOwner() == nullptr)
	{
		PlayerCharacter = nullptr;
		PlayerController = nullptr;
	}
	else
	{
		PlayerCharacter = PlayerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : PlayerCharacter;
		if (PlayerCharacter && PlayerCharacter->GetEquippedWeapon() == this)
			SetHUDAmmo();
	}
}

void AWeapon::Fire(const FVector& HitTarget)
{
	if (FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}
	if (CasingClass)
	{
		const USkeletalMeshSocket* EjectSocket = GetWeaponMesh()->GetSocketByName(FName("AmmoEject"));
		if (EjectSocket)
		{
			FTransform SocketTransform = EjectSocket->GetSocketTransform(GetWeaponMesh());
			if (UWorld* World = GetWorld())
			{
				World->SpawnActor<ABulletCasing>(CasingClass, SocketTransform.GetLocation(), SocketTransform.GetRotation().Rotator());
			}
		}
	}
	// add recoil
	if (PlayerController)
	{
		PlayerController->AddPitchInput(FMath::FRandRange(-RecoilPitchMin, -RecoilPitchMax));
	}

	SpendRound();
}

void AWeapon::Dropped()
{
	if (bIsDefaultWeapon)
	{
		Destroy();
		return;
	}
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachmentTransformRules);
	SetOwner(nullptr);
	OnRep_Owner();
}

void AWeapon::TogglePickupWidget(bool bVisible)
{
	if (bIsDefaultWeapon) return;
	if (PickupWidget) PickupWidget->SetVisibility(bVisible); 
}

void AWeapon::ToggleCollision(bool bEnabled)
{
	if (SphereComponent) SphereComponent->SetCollisionEnabled(bEnabled ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
}

void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;
	OnRep_WeaponState();
}

void AWeapon::SetHUDAmmo()
{
	PlayerCharacter = PlayerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : PlayerCharacter;
	if (PlayerCharacter)
	{
		PlayerController = PlayerController == nullptr ? PlayerCharacter->GetController<ABlasterPlayerController>() : PlayerController;
		if (PlayerController)
		{
			PlayerController->SetHUDWeaponAmmo(CurrentAmmo);
		}
	}
}

void AWeapon::AddAmmo(int32 AmmoToAdd)
{
	// locally update first, then server will replicate
	CurrentAmmo = FMath::Clamp(CurrentAmmo + AmmoToAdd, 0, MaxAmmo);
	SetHUDAmmo();
	if (HasAuthority())
	{
		MultiCastAddAmmo(AmmoToAdd);
	}
}

FVector_NetQuantize AWeapon::TraceEndWithScatter(const FVector& HitTarget) const
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr) return FVector::ZeroVector;

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();
	
	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	const FVector RandomVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	const FVector EndLoc = SphereCenter + RandomVec;
	const FVector ToEndLoc = (EndLoc - TraceStart).GetSafeNormal();

	return FVector_NetQuantize(TraceStart + ToEndLoc * TRACE_LENGTH);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	// overlap on client and server because this only serves to show the pickup widget (equip is still done on server)
    SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    SphereComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereBeginOverlap);
	SphereComponent->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);

	TogglePickupWidget(false);

}

void AWeapon::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ABlasterCharacter* Character = Cast<ABlasterCharacter>(OtherActor))
	{
		Character->SetOverlappingWeapon(this);
    }
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (ABlasterCharacter* Character = Cast<ABlasterCharacter>(OtherActor))
	{
		if (Character->GetOverlappingWeapon() == this)
			Character->SetOverlappingWeapon(nullptr);
		else
			TogglePickupWidget(false);
	}
}

void AWeapon::OnHighPing(bool bHighPing)
{
	// if ping is too high, we do not want to use server side rewind
	// Potential Issue: bUseServerSideRewind may be different on client and server (if ping is high) and damage may be applied twice
	bUseServerSideRewind = !bHighPing; 
}


void AWeapon::SpendRound()
{
	// update locally first so that the HUD is updated immediately
	CurrentAmmo = FMath::Clamp(CurrentAmmo - 1, 0, MaxAmmo);
	SetHUDAmmo();
	// server reconsiliation
	if (HasAuthority())
	{
		ClientUpdateAmmo(CurrentAmmo);
	}
	else if (PlayerCharacter && PlayerCharacter->IsLocallyControlled())
	{
		++Sequence;
	}
}

void AWeapon::ClientUpdateAmmo_Implementation(int32 ServerAmmo)
{
	if (HasAuthority()) return;
	CurrentAmmo = ServerAmmo;
	--Sequence;
	// assume the number of sequence is the same as the number of rounds fired to avoid jittering
	CurrentAmmo -= Sequence;
	SetHUDAmmo();
}

void AWeapon::MultiCastAddAmmo_Implementation(int32 AmmoToAdd)
{
	if (HasAuthority()) return;
	CurrentAmmo = FMath::Clamp(CurrentAmmo + AmmoToAdd, 0, MaxAmmo);
	if (IsFull() && PlayerCharacter && WeaponType == EWeaponType::EWT_Shotgun)
	{
		PlayerCharacter->PlayShotgunEndMontage();
	}
	SetHUDAmmo();
}

void AWeapon::OnRep_WeaponState()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		HandleEquipped();
		break;
	case EWeaponState::EWS_Secondary:
		HandleSecondary();
		break;
	case EWeaponState::EWS_Dropped:
		HandleDropped();
        break;
	}
}

void AWeapon::HandleEquipped()
{
	if (WeaponMesh == nullptr) return;
	TogglePickupWidget(false);
	ToggleCollision(false);
	ToggleCustomDepth(false);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (WeaponType == EWeaponType::EWT_SMG)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}

	if (PlayerController && HasAuthority() && !PlayerController->OnHighPing.IsBound() && bDefaultUseServerSideRewind)
	{
		PlayerController->OnHighPing.AddDynamic(this, &AWeapon::OnHighPing);
	}
}

void AWeapon::HandleDropped()
{
	if (WeaponMesh == nullptr) return;
	ToggleCollision(true);
	WeaponMesh->SetSimulatePhysics(true);
	WeaponMesh->SetEnableGravity(true);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	ToggleCustomDepth(true);

	if (PlayerController && HasAuthority() && !PlayerController->OnHighPing.IsBound() && bDefaultUseServerSideRewind)
	{
		PlayerController->OnHighPing.RemoveDynamic(this, &AWeapon::OnHighPing);
	}
}

void AWeapon::HandleSecondary()
{
	// Currently the behavior is the same as Equipped. But maybe we want to add more features for secondary weapon.
	HandleEquipped();

	if (PlayerController && HasAuthority() && !PlayerController->OnHighPing.IsBound() && bDefaultUseServerSideRewind)
	{
		PlayerController->OnHighPing.RemoveDynamic(this, &AWeapon::OnHighPing);
	}
}

void AWeapon::ToggleCustomDepth(bool bEnabled)
{
	if (WeaponMesh)
	{
        WeaponMesh->SetRenderCustomDepth(bEnabled);
    }
}


