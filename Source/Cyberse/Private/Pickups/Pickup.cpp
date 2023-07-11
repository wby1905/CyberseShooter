// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/Pickup.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

#include "Weapon/WeaponTypes.h"

APickup::APickup()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	Sphere->SetupAttachment(RootComponent);
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Sphere->SetSphereRadius(100.0f);
	Sphere->AddLocalOffset(FVector(0.0f, 0.0f, 50.0f));

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Sphere);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetRenderCustomDepth(true);
	Mesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	Mesh->MarkRenderStateDirty();
	Mesh->SetRelativeScale3D(FVector(5.f, 5.f, 5.f));

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
	PickupWidget->SetCollisionEnabled(ECollisionEnabled::NoCollision);


	NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponent"));
	NiagaraComponent->SetupAttachment(RootComponent);

}

void APickup::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		if (Sphere)
		{
			Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Sphere->OnComponentBeginOverlap.AddDynamic(this, &APickup::OnSphereBeginOverlap);
			Sphere->OnComponentEndOverlap.AddDynamic(this, &APickup::OnSphereEndOverlap);
		}
		GetWorldTimerManager().SetTimer(
			ToggleCollisionTimerHandle,
			this,
			&APickup::ToggleCollisionTimerFinished,
			ToggleCollisionTime
		);
	}

	if (Mesh)
	{
		Mesh->AddWorldRotation(FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f));
	}
}

void APickup::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}

void APickup::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}

void APickup::ToggleCollisionTimerFinished()
{
	if (Sphere)
	{
		Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
}

void APickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Mesh)
	{
		Mesh->AddWorldRotation(FRotator(0.0f, DeltaTime * BasicTurnRate, 0.0f));
	}

}

void APickup::Destroyed()
{
	if (PickupSound)
	{
        UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());
    }
	if (PickupEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			PickupEffect,
			GetActorLocation(),
			GetActorRotation()
		);
	}
	Super::Destroyed();
}

