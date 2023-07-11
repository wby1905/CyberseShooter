// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CharacterTypes/TurningInPlace.h"
#include "BlasterAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class CYBERSE_API UBlasterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
protected:
	UPROPERTY(BlueprintReadOnly, Category = "Character")
		class ABlasterCharacter* BlasterCharacter;
	UPROPERTY(BlueprintReadOnly, Category = "Character")
		float Speed;
	UPROPERTY(BlueprintReadOnly, Category = "Character")
		bool bIsInAir;
	UPROPERTY(BlueprintReadOnly, Category = "Character")
		bool bIsAccelerating;
	UPROPERTY(BlueprintReadOnly, Category = "Character")
		bool bIsCrouching;
	UPROPERTY(BlueprintReadOnly, Category = "Character")
		bool bWeaponEquipped;
	TObjectPtr<class AWeapon> EquippedWeapon;
	UPROPERTY(BlueprintReadOnly, Category = "Character")
		bool bIsCrouched;
	UPROPERTY(BlueprintReadOnly, Category = "Character")
		bool bIsAiming;
	UPROPERTY(BlueprintReadOnly, Category = "Character")
		float YawOffset;
	UPROPERTY(BlueprintReadOnly, Category = "Character")
		float Lean;
	FRotator CharacterRotationLastFrame;
	FRotator CharacterRotationThisFrame;
	FRotator DeltaRotation;

	UPROPERTY(BlueprintReadOnly, Category = "Character")
		float AO_Yaw;
	UPROPERTY(BlueprintReadOnly, Category = "Character")
		float AO_Pitch;

	UPROPERTY(BlueprintReadOnly, Category = "Character")
		FTransform LeftHandTransform;

	UPROPERTY(BlueprintReadOnly, Category = "Character")
		ETurningInPlace TurningInPlace;

	UPROPERTY(BlueprintReadOnly, Category = "Character")
		FRotator RighthandRotation;

	UPROPERTY(BlueprintReadOnly, Category = "Character")
		bool bLocallyControlled;

	UPROPERTY(BlueprintReadOnly, Category = "Character")
		bool bRotateRootBone;

	UPROPERTY(BlueprintReadOnly, Category = "Character")
		bool bElimmed;

	UPROPERTY(BlueprintReadOnly, Category = "Character")
		bool bUseLeftHandIK;

	UPROPERTY(BlueprintReadOnly, Category = "Character")
		bool bUseAimOffsets;
};
