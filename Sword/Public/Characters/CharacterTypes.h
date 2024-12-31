#pragma once

UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	ECS_Unequipped UMETA(DisplayName = "Unequipped"), // 0
	ESC_EquippedOneHandedWeapon UMETA(DisplayName = "Equipped One-Handed Weapon"), // 1
	ESC_EquippedTwoHandedWeapon UMETA(DisplayName = "Equipped Two-Handed Weapon") // 2
};

