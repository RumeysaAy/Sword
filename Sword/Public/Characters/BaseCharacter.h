// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/HitInterface.h"
#include "BaseCharacter.generated.h"

class AWeapon;
class UAttributeComponent;
class UAnimMontage;

UCLASS()
class SWORD_API ABaseCharacter : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseCharacter();
	
	// Called every frame
    virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	virtual void Attack();
	virtual void Die();
	void DirectionalHitReact(const FVector& ImpactPoint);
	virtual void HandleDamage(float DamageAmount);
	void PlayHitSound(const FVector& ImpactPoint);
	void SpawnHitParticles(const FVector& ImpactPoint);
	void DisableCapsule();
	virtual bool CanAttack();
	bool IsAlive();
	void PlayHitReactMontage(const FName& SectionName);
	virtual int32 PlayAttackMontage();
	virtual int32 PlayDeathMontage();

	UFUNCTION(BlueprintCallable)
	virtual void AttackEnd();
	
	UFUNCTION(BlueprintCallable)
	void SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled);
	
	UPROPERTY(VisibleAnywhere, Category=Weapon)
	AWeapon* EquippedWeapon;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	UAttributeComponent* Attributes;

private:
	void PlayMontageSection(UAnimMontage* Montage, const FName& SectionName);
	int32 PlayRandomMontageSection(UAnimMontage* Montage, const TArray<FName>& SectionNames);
	
	UPROPERTY(EditAnywhere, Category=Sounds)
	USoundBase* HitSound;

	UPROPERTY(EditAnywhere, Category=VisualEffects)
	UParticleSystem* HitParticles;

	UPROPERTY(EditDefaultsOnly, Category=Montages)
	UAnimMontage* AttackMontage;

	UPROPERTY(EditDefaultsOnly, Category=Montages)
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditDefaultsOnly, Category=Montages)
	UAnimMontage* DeathMontage;

	UPROPERTY(EditAnywhere, Category=Combat)
	TArray<FName> AttackMontageSections;
	
	UPROPERTY(EditAnywhere, Category=Combat)
	TArray<FName> DeathMontageSections;
};
