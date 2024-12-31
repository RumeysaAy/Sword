// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/SwordAnimInstance.h"
#include "Characters/SwordCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void USwordAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	SwordCharacter = Cast<ASwordCharacter>(TryGetPawnOwner());
	if(SwordCharacter)
	{
		SwordCharacterMovement = SwordCharacter->GetCharacterMovement();
	}
}

void USwordAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if(SwordCharacterMovement)
	{
		GroundSpeed = UKismetMathLibrary::VSizeXY(SwordCharacterMovement->Velocity);
		// karakter düşüyor mu?
		IsFalling = SwordCharacterMovement->IsFalling();
	}
}
