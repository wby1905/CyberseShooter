// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BlasterAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

#include "Weapon/Weapon.h"
#include "Character/BlasterCharacter.h"
#include "CharacterTypes/CombatState.h"

void UBlasterAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
}

void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!BlasterCharacter)
    {
        BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
        return;
    }

    Speed = BlasterCharacter->GetSpeed();
    bIsInAir = BlasterCharacter->GetMovementComponent()->IsFalling();
    bIsAccelerating = BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0;
    bIsCrouching = BlasterCharacter->GetCharacterMovement()->IsCrouching();
    bWeaponEquipped = BlasterCharacter->IsWeaponEquipped();
    EquippedWeapon = BlasterCharacter->GetEquippedWeapon();
    bIsCrouched = BlasterCharacter->bIsCrouched;
    bIsAiming = BlasterCharacter->IsAiming();
    AO_Yaw = BlasterCharacter->GetAO_Yaw();
    AO_Pitch = BlasterCharacter->GetAO_Pitch();
    TurningInPlace = BlasterCharacter->GetTurningInPlace();
    bRotateRootBone = BlasterCharacter->ShouldRotateRootBone();
    bElimmed = BlasterCharacter->IsElimmed();

    // Offset Yaw for Strafing
    FRotator AimRotation{ BlasterCharacter->GetBaseAimRotation() };
    FRotator MovementRotation{ BlasterCharacter->GetActorRotation() };
    FRotator DeltaYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
    DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaYaw, DeltaSeconds, 6.0f);
    YawOffset = DeltaRotation.Yaw;


    // Lean
    CharacterRotationLastFrame = CharacterRotationThisFrame;
    CharacterRotationThisFrame = BlasterCharacter->GetActorRotation();
    const FRotator Delta{ UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotationThisFrame, CharacterRotationLastFrame) };
    const float Target = Delta.Yaw / DeltaSeconds;
    const float Interp{FMath::FInterpTo(Lean, Target, DeltaSeconds, 6.0f)};
    Lean = FMath::Clamp(Interp, -90.0f, 90.0f);

    if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && BlasterCharacter->GetMesh())
    {
        LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
        FVector OutPosition;
        FRotator OutRotation;
        BlasterCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), LeftHandTransform.GetRotation().Rotator(), OutPosition, OutRotation);
        LeftHandTransform.SetLocation(OutPosition);
        LeftHandTransform.SetRotation(OutRotation.Quaternion());

        // only calibrate weapon ratation on the local client to optimize performance
        if (BlasterCharacter->IsLocallyControlled())
        {
            bLocallyControlled = true;
            FTransform RightHandTransform = BlasterCharacter->GetMesh()->GetSocketTransform(FName("hand_r"), ERelativeTransformSpace::RTS_World);
            // the forward vector of the character is inverted, so the look at rotation is inverted as well
            FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - BlasterCharacter->GetHitTarget()));
            RighthandRotation = UKismetMathLibrary::RInterpTo(RighthandRotation, LookAtRotation, DeltaSeconds, 30.0f);
        }
    }

    bUseLeftHandIK = BlasterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied;
    bool bFABRIKOverride = BlasterCharacter->IsLocallyControlled() &&
        BlasterCharacter->GetCombatState() != ECombatState::ECS_Throwing &&
        BlasterCharacter->GetCombatState() != ECombatState::ECS_Swapping;
    if (bFABRIKOverride)
    {
        bUseLeftHandIK = !BlasterCharacter->IsLocallyReloading() && BlasterCharacter->bFinishedSwapping;
    }
    bUseAimOffsets = BlasterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied;
}
