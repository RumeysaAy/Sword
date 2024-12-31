// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "SwordAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class SWORD_API USwordAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UPROPERTY(BlueprintReadOnly)
	class ASwordCharacter* SwordCharacter;

	UPROPERTY(BlueprintReadOnly, Category=Movement)
	class UCharacterMovementComponent* SwordCharacterMovement;

	UPROPERTY(BlueprintReadOnly, Category=Movement)
	float GroundSpeed;

	UPROPERTY(BlueprintReadOnly, Category=Movement)
	bool IsFalling;
};
