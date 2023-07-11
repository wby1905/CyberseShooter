// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

UCLASS()
class CYBERSE_API APickup : public AActor
{
	GENERATED_BODY()
	
public:	
	APickup();

	virtual void Tick(float DeltaTime) override;

	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
		virtual void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);


private:
	UPROPERTY(VisibleAnywhere)
		TObjectPtr<class UStaticMeshComponent> Mesh;
	UPROPERTY(VisibleAnywhere)
		TObjectPtr<class USphereComponent> Sphere;
	UPROPERTY(VisibleAnywhere)
		TObjectPtr<class UWidgetComponent> PickupWidget;
	UPROPERTY(EditDefaultsOnly)
		TObjectPtr<class USoundBase> PickupSound;
	UPROPERTY(VisibleAnywhere)
		TObjectPtr<class UNiagaraComponent> NiagaraComponent;
	UPROPERTY(EditDefaultsOnly)
		TObjectPtr<class UNiagaraSystem> PickupEffect;


	UPROPERTY(EditDefaultsOnly)
		float BasicTurnRate = 45.0f;

	FTimerHandle ToggleCollisionTimerHandle;
	UPROPERTY(EditDefaultsOnly)
        float ToggleCollisionTime = 0.25f;
	void ToggleCollisionTimerFinished();

};
