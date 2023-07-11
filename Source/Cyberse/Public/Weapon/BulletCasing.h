// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BulletCasing.generated.h"

UCLASS()
class CYBERSE_API ABulletCasing : public AActor
{
	GENERATED_BODY()
	
public:	
	ABulletCasing();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
		virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:
	UPROPERTY(EditDefaultsOnly)
		TObjectPtr<UStaticMeshComponent> CasingMesh;

	UPROPERTY(EditDefaultsOnly)
		float EjectionImpulse = 10.0f;

	UPROPERTY(EditDefaultsOnly)
		float LifeSpan = 3.0f;

	UPROPERTY(EditDefaultsOnly)
		TObjectPtr<class USoundBase> CasingSound;

	bool bHasHit = false;

};
