// Fill out your copyright notice in the Description page of Project Settings.


#include "CyberseComponents/LagCompensationComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"

#include "Character/BlasterCharacter.h"
#include "Weapon/Weapon.h"
#include "../Cyberse.h"

#include "DrawDebugHelpers.h"

ULagCompensationComponent::ULagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	SaveFramePackage();
}

void ULagCompensationComponent::ShowFramePackage(const FFramePackage& Package, const FColor& Color)
{
	for (auto& BoxPair : Package.HitBoxInfo)
    {
        DrawDebugBox(GetWorld(), BoxPair.Value.Location, BoxPair.Value.BoxExtent, BoxPair.Value.Rotation.Quaternion(), Color, false, 5.f);
    }
}

void ULagCompensationComponent::ServerScoreRequest_Implementation(ABlasterCharacter* HitCharacter, const FVector_NetQuantize100& TraceStart, const FVector_NetQuantize100& HitLocation, float HitTime, AWeapon* DamageCauser)
{
	FServerSideRewindResult Result = ServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime);

	if (OwnerCharacter && HitCharacter && DamageCauser && Result.bHitConfirmed)
	{
		float Damage = DamageCauser->GetDamage();
		if (Result.bHeadShot)
		{
            Damage *= DamageCauser->GetHeadshotMultiplier();
        }
		UGameplayStatics::ApplyDamage(
			HitCharacter,
			Damage,
			OwnerCharacter->GetController(),
			DamageCauser,
			UDamageType::StaticClass()
		);
	}
}

FServerSideRewindResult ULagCompensationComponent::ServerSideRewind(ABlasterCharacter* HitCharacter, const FVector_NetQuantize100& TraceStart, const FVector_NetQuantize100& HitLocation, float HitTime)
{
	FFramePackage TargetFrame = GetFramePackage(HitCharacter, HitTime);
	if (TargetFrame.HitBoxInfo.Num() == 0)
	{
		return FServerSideRewindResult{ false, false };
    }
	if (bDebug)	ShowFramePackage(TargetFrame, FColor::Red);
	return ConfirmHit(TargetFrame, HitCharacter, TraceStart, HitLocation);
}

void ULagCompensationComponent::ServerProjectileScoreRequest_Implementation(ABlasterCharacter* HitCharacter, const FVector_NetQuantize100& TraceStart, const FVector_NetQuantize100& LaunchVelocity, float HitTime, AWeapon* DamageCauser)
{
	FServerSideRewindResult Result = ProjectileServerSideRewind(HitCharacter, TraceStart, LaunchVelocity, HitTime);

	if (OwnerCharacter && HitCharacter && DamageCauser && Result.bHitConfirmed)
	{
		float Damage = DamageCauser->GetDamage();
		if (Result.bHeadShot)
		{
            Damage *= DamageCauser->GetHeadshotMultiplier();
        }
		UGameplayStatics::ApplyDamage(
            HitCharacter,
            Damage,
            OwnerCharacter->GetController(),
            DamageCauser,
            UDamageType::StaticClass()
        );
    }
}

FServerSideRewindResult ULagCompensationComponent::ProjectileServerSideRewind(ABlasterCharacter* HitCharacter, const FVector_NetQuantize100& TraceStart, const FVector_NetQuantize100& LaunchVelocity, float HitTime)
{
	FFramePackage TargetFrame = GetFramePackage(HitCharacter, HitTime);
	if (TargetFrame.HitBoxInfo.Num() == 0)
	{
		return FServerSideRewindResult{ false, false };
	}
	if (bDebug)	ShowFramePackage(TargetFrame, FColor::Red);
	return ProjectileConfirmHit(TargetFrame, HitCharacter, TraceStart, LaunchVelocity);
}

void ULagCompensationComponent::ServerShotgunScoreRequest_Implementation(const TArray<ABlasterCharacter*>& HitCharacters, const FVector_NetQuantize100& TraceStart, const TArray<FVector_NetQuantize100>& HitLocations, float HitTime, AWeapon* DamageCauser)
{
	FShotgunServerSideRewindResult ConfirmResult = ShotgunServerSideRewind(HitCharacters, TraceStart, HitLocations, HitTime);

	if (OwnerCharacter && DamageCauser)
	{
		for (auto& HitCharacter : HitCharacters)
		{
			if (HitCharacter == nullptr) continue;
			float TotalDamage = 0.f;
			if (ConfirmResult.HeadShots.Contains(HitCharacter))
			{
				TotalDamage += ConfirmResult.HeadShots[HitCharacter] * DamageCauser->GetDamage() * DamageCauser->GetHeadshotMultiplier();
			}
			if (ConfirmResult.BodyShots.Contains(HitCharacter))
			{
                TotalDamage += ConfirmResult.BodyShots[HitCharacter] * DamageCauser->GetDamage();
            }
			if (TotalDamage > 0.f)
			{
				UGameplayStatics::ApplyDamage(
					HitCharacter,
					TotalDamage,
					OwnerCharacter->GetController(),
					DamageCauser,
					UDamageType::StaticClass()
				);
			}
		}
    }
}

FShotgunServerSideRewindResult ULagCompensationComponent::ShotgunServerSideRewind(const TArray<ABlasterCharacter*>& HitCharacters, const FVector_NetQuantize100& TraceStart, const TArray<FVector_NetQuantize100>& HitLocations, float HitTime)
{
	TArray<FFramePackage> TargetFrames;
	for (ABlasterCharacter* HitCharacter : HitCharacters)
	{
		if (HitCharacter == nullptr) continue;
		FFramePackage TargetFrame = GetFramePackage(HitCharacter, HitTime);
		if (TargetFrame.HitBoxInfo.Num() == 0)
		{
            continue;
        }
		TargetFrames.Add(TargetFrame);
		if (bDebug) ShowFramePackage(TargetFrame, FColor::Red);
	}
	return ShotgunConfirmHit(HitCharacters, TargetFrames, TraceStart, HitLocations);
}

void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerCharacter = Cast<ABlasterCharacter>(GetOwner());
}

FServerSideRewindResult ULagCompensationComponent::ConfirmHit(const FFramePackage& Package, ABlasterCharacter* HitCharacter, const FVector_NetQuantize100& TraceStart, const FVector_NetQuantize100& HitLocation)
{
	if (HitCharacter == nullptr) return FServerSideRewindResult();

	FFramePackage CurrentFrame;
	// cache current frame to move back later
	CacheBoxPositions(HitCharacter, CurrentFrame);
	MoveBoxes(HitCharacter, Package, true);

	// check head shot
	UBoxComponent* HeadBox = HitCharacter->GetHitBoxes()[FName("head")];

	FHitResult ConfirmHitResult;
	FServerSideRewindResult Result{ false, false };
	Result.bHitConfirmed = false;
	Result.bHeadShot = false;
	const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
	UWorld* World = GetWorld();
	if (World)
	{
		World->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, TraceEnd, ECC_HitBox);

		if (ConfirmHitResult.bBlockingHit && ConfirmHitResult.GetActor() == HitCharacter)
		{
			DrawHitPoint(World, ConfirmHitResult);
			Result.bHitConfirmed = true;
			if (HeadBox && ConfirmHitResult.GetComponent() == HeadBox)
			{
				// head shot
				Result.bHeadShot = true;
			}
		}
	}
	MoveBoxes(HitCharacter, CurrentFrame, false);
	return Result;
}

FServerSideRewindResult ULagCompensationComponent::ProjectileConfirmHit(const FFramePackage& Package, ABlasterCharacter* HitCharacter, const FVector_NetQuantize100& TraceStart, const FVector_NetQuantize100& LaunchVelocity)
{
	if (HitCharacter == nullptr) return FServerSideRewindResult();
	FFramePackage CurrentFrame;
	// cache current frame to move back later
	CacheBoxPositions(HitCharacter, CurrentFrame);
	MoveBoxes(HitCharacter, Package, true);

	// check head shot
	UBoxComponent* HeadBox = HitCharacter->GetHitBoxes()[FName("head")];

	FServerSideRewindResult Result{ false, false };

	FPredictProjectilePathParams PathParams;
	PathParams.StartLocation = TraceStart;
	PathParams.LaunchVelocity = LaunchVelocity;
	PathParams.bTraceWithCollision = true;
	PathParams.bTraceWithChannel = true;
	PathParams.TraceChannel = ECC_HitBox;
	PathParams.MaxSimTime = MaxRecordTime;
	PathParams.SimFrequency = 15.f;
	PathParams.ProjectileRadius = 5.f;
	PathParams.ActorsToIgnore.Add(GetOwner());
	if (bDebug)
	{
		PathParams.DrawDebugType = EDrawDebugTrace::ForDuration;
		PathParams.DrawDebugTime = 5.f;
	}

	FPredictProjectilePathResult PathResult;
	UGameplayStatics::PredictProjectilePath(GetWorld(), PathParams, PathResult);

	if (PathResult.HitResult.bBlockingHit)
	{
		FHitResult ConfirmHitResult = PathResult.HitResult;
		DrawHitPoint(GetWorld(), ConfirmHitResult);
		Result.bHitConfirmed = true;
		if (HeadBox && ConfirmHitResult.GetComponent() == HeadBox)
		{
            // head shot
            Result.bHeadShot = true;
        }
	}
	MoveBoxes(HitCharacter, CurrentFrame, false);
	return Result;	
}


FShotgunServerSideRewindResult ULagCompensationComponent::ShotgunConfirmHit(const TArray<ABlasterCharacter*>& HitCharacters, const TArray<FFramePackage>& Packages, const FVector_NetQuantize100& TraceStart, const TArray<FVector_NetQuantize100>& HitLocations)
{
	// length must be the same
	if (HitCharacters.Num() != Packages.Num() || HitCharacters.Num() <= 0)
	{
        return FShotgunServerSideRewindResult();
    }
	FShotgunServerSideRewindResult ShotgunResult;

	// Enable collision for all characters' hit boxes (otherwise overlapped characters might get hit)
	for (int8 i = 0; i < HitCharacters.Num(); ++i)
	{
		MoveBoxes(HitCharacters[i], Packages[i], true);
    }
	
	for (int8 i = 0; i < HitCharacters.Num(); ++i)
	{
		if (HitCharacters[i] == nullptr) continue;
		for (int8 j = 0; j < HitLocations.Num(); ++j)
		{
			FServerSideRewindResult Result = ConfirmHit(Packages[i], HitCharacters[i], TraceStart, HitLocations[j]);
			if (Result.bHitConfirmed)
			{
				if (Result.bHeadShot)
				{
					uint32 Count = ShotgunResult.HeadShots.FindOrAdd(HitCharacters[i], 0);
					ShotgunResult.HeadShots[HitCharacters[i]] = Count + 1;
				}
				else
				{
					uint32 Count = ShotgunResult.BodyShots.FindOrAdd(HitCharacters[i], 0);
					ShotgunResult.BodyShots[HitCharacters[i]] = Count + 1;
				}
			}

		}
		// re-enable collision (disabled in ConfirmHit)
		MoveBoxes(HitCharacters[i], Packages[i], true);
    }

	// Disable collisions
	for (int8 i = 0; i < HitCharacters.Num(); ++i)
	{
		MoveBoxes(HitCharacters[i], Packages[i], false);
    }

	return ShotgunResult;
}

FFramePackage ULagCompensationComponent::LerpBetweenFrames(const FFramePackage& A, const FFramePackage& B, float Time)
{
	// bounds check to ensure the return is reasonable
	if (Time < A.Time) return A;
	if (Time > B.Time) return B;
	if (A.Time > B.Time) return LerpBetweenFrames(B, A, Time);
	if (A.Time == B.Time) return A;
	FFramePackage Result;
	const float Alpha = (Time - A.Time) / (B.Time - A.Time);
	for (auto& BoxPair : A.HitBoxInfo)
    {
		const FName& Key = BoxPair.Key;
		const FBoxInfo& BoxA = BoxPair.Value;
		const FBoxInfo& BoxB = B.HitBoxInfo[Key];
        FBoxInfo BoxInfo;
        BoxInfo.Location = FMath::Lerp(BoxA.Location, BoxB.Location, Alpha);
        BoxInfo.Rotation = FMath::LerpRange(BoxA.Rotation, BoxB.Rotation, Alpha);
		// extent is the same
		BoxInfo.BoxExtent = BoxA.BoxExtent;
        Result.HitBoxInfo.Add(Key, BoxInfo);
    }
	return Result;
}

FFramePackage ULagCompensationComponent::GetFramePackage(ABlasterCharacter* Character, float Time)
{
	bool bReturn =
		Character == nullptr ||
		Character->GetLagCompensationComponent() == nullptr ||
		Character->GetLagCompensationComponent()->FrameHistory.Num() < 1;
	if (bReturn) return FFramePackage();
	FFramePackage TargetFrame;
	const TDoubleLinkedList<FFramePackage>& CharacterHistory = Character->GetLagCompensationComponent()->FrameHistory;

	// check bounds
	const float OldestTime = CharacterHistory.GetHead()->GetValue().Time;
	const float NewestTime = CharacterHistory.GetTail()->GetValue().Time;
	// too laggy to do SSR
	if (Time < OldestTime) return FFramePackage();

	// search for the first frame that is older than Time
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Node = CharacterHistory.GetTail();
	while (Node->GetValue().Time > Time)
	{
		if (Node->GetPrevNode() == nullptr) break;
		Node = Node->GetPrevNode();
	}
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* NextNode = Node->GetNextNode();
	// if it is too new
	if (NextNode == nullptr) NextNode = Node;
	// do linear interpolation
	TargetFrame = LerpBetweenFrames(Node->GetValue(), NextNode->GetValue(), Time);
	return TargetFrame;
}

void ULagCompensationComponent::CacheBoxPositions(ABlasterCharacter* Character, FFramePackage& OutPackage)
{
	if (Character == nullptr) return;
	for (auto& BoxPair : Character->GetHitBoxes())
	{
		if (BoxPair.Value == nullptr) continue;
        FBoxInfo BoxInfo;
        BoxInfo.Location = BoxPair.Value->GetComponentLocation();
        BoxInfo.Rotation = BoxPair.Value->GetComponentRotation();
        BoxInfo.BoxExtent = BoxPair.Value->GetScaledBoxExtent();
        OutPackage.HitBoxInfo.Add(BoxPair.Key, BoxInfo);
    }
}

void ULagCompensationComponent::MoveBoxes(ABlasterCharacter* Character, const FFramePackage& Package, bool bCollisionEnabled)
{
	if (Character == nullptr) return;
	for (auto& BoxPair : Character->GetHitBoxes())
	{
		if (BoxPair.Value == nullptr) continue;
		const FBoxInfo& BoxInfo = Package.HitBoxInfo[BoxPair.Key];
		BoxPair.Value->SetWorldLocation(BoxInfo.Location);
		BoxPair.Value->SetWorldRotation(BoxInfo.Rotation);
		BoxPair.Value->SetBoxExtent(BoxInfo.BoxExtent);
		if (bCollisionEnabled)
		{
			BoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
		else
		{
			BoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
    }
}

void ULagCompensationComponent::SaveFramePackage()
{
	if (OwnerCharacter == nullptr || !OwnerCharacter->HasAuthority()) return;
	// server itself doesn't need to save frame
	if (OwnerCharacter->GetRemoteRole() != ROLE_AutonomousProxy) return;

	FFramePackage ThisFrame;
	SaveFramePackage(ThisFrame);
	FrameHistory.AddTail(ThisFrame);

	if (FrameHistory.Num() > 5)
	{
		float HistoryLength = FrameHistory.GetTail()->GetValue().Time - FrameHistory.GetHead()->GetValue().Time;
		while (HistoryLength > MaxRecordTime)
		{
			FrameHistory.RemoveNode(FrameHistory.GetHead());
			HistoryLength = FrameHistory.GetTail()->GetValue().Time - FrameHistory.GetHead()->GetValue().Time;
		}
	}

}

void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
	OwnerCharacter = OwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : OwnerCharacter;
	if (OwnerCharacter)
	{
		Package.Time = GetWorld()->GetTimeSeconds();
		CacheBoxPositions(OwnerCharacter, Package);
	}
}

void ULagCompensationComponent::DrawHitPoint(UWorld* World, FHitResult& ConfirmHitResult)
{
	if (bDebug)
	{
		DrawDebugSphere(World, ConfirmHitResult.ImpactPoint, 10.0f, 12, FColor::Green, false, 5.f);
		if (ConfirmHitResult.GetComponent())
		{
			UBoxComponent* HitBox = Cast<UBoxComponent>(ConfirmHitResult.GetComponent());
			if (HitBox)
			{
				DrawDebugBox(World, HitBox->GetComponentLocation(), HitBox->GetScaledBoxExtent(), HitBox->GetComponentQuat(), FColor::Green, false, 5.f);
			}
		}
	}
}
