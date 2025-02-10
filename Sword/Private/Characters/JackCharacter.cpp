// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/JackCharacter.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Items/Item.h"
#include "Items/Weapons/Weapon.h"
#include "Animation/AnimMontage.h"

// Sets default values
AJackCharacter::AJackCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch= false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	
	// karakter gidilen yöne doğru dönecek
	GetCharacterMovement()->bOrientRotationToMovement = true;
	// dönme hızı
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->TargetArmLength = 300.f;
	
	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(SpringArm);
}

// Called every frame
void AJackCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AJackCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &AJackCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AJackCharacter::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(EKeyAction, ETriggerEvent::Triggered, this, &AJackCharacter::EKeyPressed);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &AJackCharacter::Attack);
	}

	// PlayerInputComponent->BindAction(FName("Jump"), IE_Pressed, this, &ACharacter::Jump);
	// PlayerInputComponent->BindAction(FName("Equip"), IE_Pressed, this, &AJackCharacter::EKeyPressed);
	// PlayerInputComponent->BindAction(FName("Attack"), IE_Pressed, this, &AJackCharacter::Attack);
}

// Called when the game starts or when spawned
void AJackCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if(UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(SwordContext, 0);
		}
	}

	Tags.Add(FName("JackCharacter"));
}

void AJackCharacter::Move(const FInputActionValue& Value)
{
	// hiçbir animasyon oynatılmıyorsa ilerleyebilsin
	if (ActionState != EActionState::EAS_Unoccupied) return;
	
	const FVector2D MovementVector = Value.Get<FVector2D>();

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);
	
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(ForwardDirection, MovementVector.Y);

	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void AJackCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	
	AddControllerPitchInput(LookAxisVector.Y);
	AddControllerYawInput(LookAxisVector.X);
}

void AJackCharacter::EKeyPressed()
{
	AWeapon* OverlappingWeapon = Cast<AWeapon>(OverlappingItem);
	if(OverlappingWeapon)
	{
		EquipWeapon(OverlappingWeapon);
	}
	else
	{
		// hiçbir şey yapmıyorsa ve silahı varsa 
		if (CanDisarm())
		{
			// silahı sırtına koyabilir
			Disarm();
		}
		else if (CanArm())
		{
			// silahı sırtından alacak
			Arm();
		}
	}
}

void AJackCharacter::Attack()
{
	Super::Attack();
	PlayAttackMontage();
	
	// hiçbir şey yapmıyorsa ve silahı varsa attack animasyonu oynatılacak
	if (CanAttack())
	{
		PlayAttackMontage();
		ActionState = EActionState::EAS_Attacking;
	}
}

void AJackCharacter::EquipWeapon(AWeapon* Weapon)
{
	// silahı topladı - silah elinde
	Weapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
	CharacterState = ECharacterState::ESC_EquippedOneHandedWeapon;
	OverlappingItem = nullptr;
	EquippedWeapon = Weapon;
}

void AJackCharacter::AttackEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}

bool AJackCharacter::CanAttack()
{
	return ActionState == EActionState::EAS_Unoccupied &&
	CharacterState != ECharacterState::ECS_Unequipped;
}

bool AJackCharacter::CanDisarm()
{
	return ActionState == EActionState::EAS_Unoccupied &&
		CharacterState != ECharacterState::ECS_Unequipped;
}

bool AJackCharacter::CanArm()
{
	// hiçbir şey yapmıyorsa ve silah sırtındaysa ve silah varsa
	return ActionState == EActionState::EAS_Unoccupied &&
		CharacterState == ECharacterState::ECS_Unequipped && EquippedWeapon;
}

void AJackCharacter::Disarm()
{
	// silahı sırtına koyabilir
	PlayEquipMontage(FName("Unequip"));
	CharacterState = ECharacterState::ECS_Unequipped;
	ActionState = EActionState::EAS_EquippingWeapon;
}

void AJackCharacter::Arm()
{
	// silahı sırtından alacak
	PlayEquipMontage(FName("Equip"));
	CharacterState = ECharacterState::ESC_EquippedOneHandedWeapon;
	ActionState = EActionState::EAS_EquippingWeapon;
}

void AJackCharacter::PlayEquipMontage(const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && EquipMontage)
	{
		AnimInstance->Montage_Play(EquipMontage);
		AnimInstance->Montage_JumpToSection(SectionName, EquipMontage);
	}
}

void AJackCharacter::AttachWeaponToBack()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("SpineSocket"));
	}
}

void AJackCharacter::AttachWeaponToHand()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("RightHandSocket"));
	}
}

void AJackCharacter::FinishEquipping()
{
	ActionState = EActionState::EAS_Unoccupied;
}












