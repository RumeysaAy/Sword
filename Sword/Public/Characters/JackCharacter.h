// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "InputActionValue.h"
#include "CharacterTypes.h"
#include "JackCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
class USpringArmComponent;
class UCameraComponent;
class AItem;
class UAnimMontage;

UCLASS()
class SWORD_API AJackCharacter : public ABaseCharacter
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

	UPROPERTY(EditAnywhere, Category=Input)
	UInputAction* EKeyAction;

	UPROPERTY(EditAnywhere, Category=Input)
	UInputAction* AttackAction;

	/** Callbacks for input */ 
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	void EKeyPressed();
	virtual void Attack() override;

	/** Combat - Play montage functions */
	void EquipWeapon(AWeapon* Weapon);
	
	virtual void AttackEnd() override;

	virtual bool CanAttack() override;
	
	bool CanDisarm();
	bool CanArm();
	void Disarm();
	void Arm();
	void PlayEquipMontage(const FName& SectionName);

	UFUNCTION(BlueprintCallable)
	void AttachWeaponToBack();

	UFUNCTION(BlueprintCallable)
	void AttachWeaponToHand();

	UFUNCTION(BlueprintCallable)
	void FinishEquipping();

private:
	/** Character Components */
	
	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* ViewCamera;

	UPROPERTY(VisibleInstanceOnly)
	AItem* OverlappingItem;

	/*
	 * Animation Montages
	 */

	UPROPERTY(EditDefaultsOnly, Category=Montages)
	UAnimMontage* EquipMontage;

	ECharacterState CharacterState = ECharacterState::ECS_Unequipped;

	UPROPERTY(BlueprintReadWrite, meta=(AllowPrivateAccess = "true"))
	EActionState ActionState = EActionState::EAS_Unoccupied;

public:
	FORCEINLINE void SetOverlappingItem(AItem* Item) { OverlappingItem = Item; }
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }
};
