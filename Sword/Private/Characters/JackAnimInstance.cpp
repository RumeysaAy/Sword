// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/JackAnimInstance.h"
#include "Characters/JackCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UJackAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	JackCharacter = Cast<AJackCharacter>(TryGetPawnOwner());

	if (JackCharacter)
	{
		JackCharacterMovement = JackCharacter->GetCharacterMovement();
	}
}

void UJackAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (JackCharacterMovement)
	{
		GroundSpeed = UKismetMathLibrary::VSizeXY(JackCharacterMovement->Velocity);
		IsFalling = JackCharacterMovement->IsFalling();
		CharacterState = JackCharacter->GetCharacterState();
	}
}
