#pragma once

UENUM(BlueprintType)
enum class ETurningInPlace : uint8
{
    ETIP_NotTurning UMETA(DisplayName = "NotTurning"),
    ETIP_Left UMETA(DisplayName = "Turning Left"),
    ETIP_Right UMETA(DisplayName = "Turning Right"),

    ETIP_MAX UMETA(DisplayName = "DefaultMAX")
};