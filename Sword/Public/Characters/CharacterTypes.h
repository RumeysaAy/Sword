#pragma once

UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	ECS_Unequipped UMETA(DisplayName = "Unequipped"), // 0
	ESC_EquippedOneHandedWeapon UMETA(DisplayName = "Equipped One-Handed Weapon"), // 1
	ESC_EquippedTwoHandedWeapon UMETA(DisplayName = "Equipped Two-Handed Weapon") // 2
};

UENUM(BlueprintType)
enum class EActionState : uint8
{
	EAS_Unoccupied UMETA(Displayname = "Unoccupied"),
	EAS_Attacking UMETA(DisplayName = "Attacking"),
	EAS_EquippingWeapon UMETA(DisplayName = "Equipping Weapon")
};

