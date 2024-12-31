// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "CharacterTypes.h"
#include "JackCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
class USpringArmComponent;
class UCameraComponent;
class AItem;

UCLASS()
class SWORD_API AJackCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AJackCharacter();
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category=Input)
	UInputMappingContext* SwordContext;

	UPROPERTY(EditAnywhere, Category=Input)
	UInputAction* MovementAction;

	UPROPERTY(EditAnywhere, Category=Input)
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category=Input)
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere,Category=Input)
	UInputAction* EKeyAction;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	void EKeyPressed();

private:
	ECharacterState CharacterState = ECharacterState::ECS_Unequipped;
	
	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* ViewCamera;

	UPROPERTY(VisibleInstanceOnly)
	AItem* OverlappingItem;

public:
	FORCEINLINE void SetOverlappingItem(AItem* Item) { OverlappingItem = Item; }
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }
};
