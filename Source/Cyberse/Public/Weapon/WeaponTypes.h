#pragma once

#define TRACE_LENGTH 80000.f

#define CUSTOM_DEPTH_PURPLE 250
#define CUSTOM_DEPTH_BLUE 251
#define CUSTOM_DEPTH_TAN 252


UENUM(BlueprintType)
enum class EWeaponType : uint8
{
    EWT_Rifle UMETA(DisplayName = "Rifle"),
    EWT_RocketLauncher UMETA(DisplayName = "RocketLauncher"),
    EWT_Pistol UMETA(DisplayName = "Pistol"),
    EWT_SMG UMETA(DisplayName = "SMG"),
    EWT_Shotgun UMETA(DisplayName = "Shotgun"),
    EWT_SniperRifle UMETA(DisplayName = "SniperRifle"),
    EWT_GrenadeLauncher UMETA(DisplayName = "GrenadeLauncher"),

    EWT_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EFireType : uint8
{
    EFT_HitScan UMETA(DisplayName = "HitScan"),
    EFT_Projectile UMETA(DisplayName = "Projectile"),
    EFT_Shotgun UMETA(DisplayName = "Shotgun"),

    EFT_MAX UMETA(DisplayName = "DefaultMAX")
};