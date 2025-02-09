// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Enemy.h"

#include "AIController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "Components/AttributeComponent.h"
#include "HUD/HealthBarComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Characters/JackCharacter.h"
#include "Items/Weapons/Weapon.h"
#include "TimerManager.h"

#include "Sword/DebugMacro.h"


// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->SetCollisionObjectType(ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetGenerateOverlapEvents(true);
	
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera,ECR_Ignore);
	
	HealthBarWidget = CreateDefaultSubobject<UHealthBarComponent>(TEXT("HealthBar"));
	HealthBarWidget->SetupAttachment(GetRootComponent());

	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	PawnSensing = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensing"));
	PawnSensing->SightRadius = 4000.f;
	PawnSensing->SetPeripheralVisionAngle(45.f);
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	if(HealthBarWidget) HealthBarWidget->SetVisibility(false);

	EnemyController = Cast<AAIController>(GetController());
	MoveToTarget(PatrolTarget);

	if(PawnSensing)
	{
		PawnSensing->OnSeePawn.AddDynamic(this, &AEnemy::PawnSeen);
	}

	UWorld* World = GetWorld();
	if (World && WeaponClass)
	{
		AWeapon* DefaultWeapon = World->SpawnActor<AWeapon>(WeaponClass);
		DefaultWeapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
		EquippedWeapon = DefaultWeapon;
	}
}

void AEnemy::Die()
{
	EnemyState = EEnemyState::EES_Dead;
	PlayDeathMontage();
	ClearAttackTimer();
	
	HideHealthBar();
	DisableCapsule();
	SetLifeSpan(DeathLifeSpan);

	GetCharacterMovement()->bOrientRotationToMovement = false;
}

bool AEnemy::InTargetRange(AActor* Target, double Radius)
{
	if (Target == nullptr) return false;
	const double DistanceToTarget = (Target->GetActorLocation() - GetActorLocation()).Size();
	DRAW_SPHERE_SingleFrame(GetActorLocation());
	DRAW_SPHERE_SingleFrame(Target->GetActorLocation());
	return Radius >= DistanceToTarget;
}

void AEnemy::MoveToTarget(AActor* Target)
{
	if (EnemyController == nullptr || Target == nullptr) return;
	
	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalActor(Target);
	MoveRequest.SetAcceptanceRadius(60.f);
		
	EnemyController->MoveTo(MoveRequest);
}

AActor* AEnemy::ChoosePatrolTarget()
{
	TArray<AActor*> ValidTargets;
	for (AActor* Target : PatrolTargets)
	{
		if (Target != PatrolTarget)
		{
			ValidTargets.AddUnique(Target);
		}
	}
			
	// düşman rastgele bir diğer hedef noktaya gider
	const int32 NumPatrolTargets = ValidTargets.Num();
	if (NumPatrolTargets > 0)
	{
		const int32 TargetSelection = FMath::RandRange(0, NumPatrolTargets-1);
		return ValidTargets[TargetSelection];
	}
	
	return nullptr;
}

void AEnemy::PawnSeen(APawn* SeenPawn)
{
	// yakalanan piyon oyuncu mu? Cast<AJackCharacter>(SeenPawn)
	const bool bShouldChaseTarget =
		EnemyState != EEnemyState::EES_Dead &&
			EnemyState != EEnemyState::EES_Chasing &&
				EnemyState < EEnemyState::EES_Attacking &&
					SeenPawn->ActorHasTag(FName("JackCharacter"));

	if (bShouldChaseTarget)
	{
		// düşman oyuncuyu gördüğünde takip edecek, devriye gezmeyecek
		CombatTarget = SeenPawn;
		ClearPatrolTimer();
		ChaseTarget();
	}
}

void AEnemy::Attack()
{
	Super::Attack();
	PlayAttackMontage();
}

bool AEnemy::CanAttack()
{
	bool bCanAttack =
		IsInsideAttackRadius() &&
		!IsAttacking() &&
		!IsDead();
	return bCanAttack;
}

void AEnemy::HandleDamage(float DamageAmount)
{
	Super::HandleDamage(DamageAmount);

	if (HealthBarWidget)
	{
		HealthBarWidget->SetHealthPercent(Attributes->GetHealthPercent());
	}
}

int32 AEnemy::PlayDeathMontage()
{
	const int32 Selection = Super::PlayDeathMontage();
	TEnumAsByte<EDeathPose> Pose(Selection);
	if (Pose < EDeathPose::EDP_MAX)
	{
		DeathPose = Pose;
	}

	return Selection;
}

void AEnemy::PatrolTimerFinished()
{
	MoveToTarget(PatrolTarget);
}

void AEnemy::HideHealthBar()
{
	if (HealthBarWidget) HealthBarWidget->SetVisibility(false);
}

void AEnemy::ShowHealthBar()
{
	if (HealthBarWidget) HealthBarWidget->SetVisibility(true);
}

void AEnemy::LoseInterest()
{
	// Outside combat radius, lose interest
	CombatTarget = nullptr;
	// düşmanın sağlık barı görünmez
	HideHealthBar();
}

void AEnemy::StartPatrolling()
{
	// oyuncu, CombatRadius'un içerisinde değilse düşman oyuncuyu takip etmeyi bırakır
	EnemyState = EEnemyState::EES_Patrolling;
	GetCharacterMovement()->MaxWalkSpeed = PatrollingSpeed;
	MoveToTarget(PatrolTarget);
}

void AEnemy::ChaseTarget()
{
	// oyuncu AttackRadius'dan çıkarsa düşman oyuncuyu takip etsin
	// Outside attack range, chase character
	EnemyState = EEnemyState::EES_Chasing;
	GetCharacterMovement()->MaxWalkSpeed = ChasingSpeed;
	MoveToTarget(CombatTarget);
}

bool AEnemy::IsOutsideCombatRadius()
{
	return !InTargetRange(CombatTarget, CombatRadius);
}

bool AEnemy::IsOutsideAttackRadius()
{
	return !InTargetRange(CombatTarget, AttackRadius);
}

bool AEnemy::IsInsideAttackRadius()
{
	return InTargetRange(CombatTarget, AttackRadius);
}

bool AEnemy::IsChasing()
{
	return EnemyState == EEnemyState::EES_Chasing;
}

bool AEnemy::IsAttacking()
{
	return EnemyState == EEnemyState::EES_Attacking;
}

bool AEnemy::IsDead()
{
	return EnemyState == EEnemyState::EES_Dead;
}

bool AEnemy::IsEngaged()
{
	return EnemyState == EEnemyState::EES_Engaged;
}

void AEnemy::ClearPatrolTimer()
{
	GetWorldTimerManager().ClearTimer(PatrolTimer);
}

void AEnemy::StartAttackTimer()
{
	EnemyState = EEnemyState::EES_Attacking;
	const float AttackTime = FMath::RandRange(AttackMin, AttackMax);
	GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime);
}

void AEnemy::ClearAttackTimer()
{
	GetWorldTimerManager().ClearTimer(AttackTimer);
}

// weapon çarptığında çağrılır
void AEnemy::GetHit_Implementation(const FVector& ImpactPoint)
{
	// düşmanın vurulduğu noktada
	// DRAW_SPHERE_COLOR(ImpactPoint, FColor::Turquoise);

	ShowHealthBar();
	
	if (IsAlive())
	{
		DirectionalHitReact(ImpactPoint);
	}
	else Die();

	PlayHitSound(ImpactPoint);
	SpawnHitParticles(ImpactPoint);
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	HandleDamage(DamageAmount);
	CombatTarget = EventInstigator->GetPawn();
	ChaseTarget();
	return DamageAmount;
}

void AEnemy::Destroyed()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Destroy();
	}
}

void AEnemy::CheckCombatTarget()
{
	// oyuncu düşmandan uzaklaştığında
	if (IsOutsideCombatRadius())
	{
		ClearAttackTimer();
		
		// Outside combat radius, lose interest
		LoseInterest();

		// düşman oyuncuya saldırmıyorsa
		if (!IsEngaged())
		{
			// oyuncu, CombatRadius'un içerisinde değilse düşman oyuncuyu takip etmeyi bırakır
			StartPatrolling();
		}
	}
	else if (IsOutsideAttackRadius() && !IsChasing())
	{
		ClearAttackTimer();
		// oyuncu AttackRadius'dan çıkarsa düşman oyuncuyu takip etsin
		// Outside attack range, chase character
		if (!IsEngaged()) ChaseTarget();
	}
	else if (CanAttack())
	{
		// oyuncu AttackRadius içerisindeyse düşman saldırır
		// Inside attack range, attack character
		StartAttackTimer();
	}
}

void AEnemy::CheckPatrolTarget()
{
	// PatrolTarget, PatrolRadius'un içerisindeyse
	if (InTargetRange(PatrolTarget, PatrolRadius))
	{
		PatrolTarget = ChoosePatrolTarget();

		const float WaitTime = FMath::RandRange(WaitMin, WaitMax);
		
		// WaitTime saniye sonra diğer bir hedefe gidecek
		GetWorldTimerManager().SetTimer(PatrolTimer, this, &AEnemy::PatrolTimerFinished, WaitTime);
	}
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsDead()) return;

	if (EnemyState > EEnemyState::EES_Patrolling)
	{
		CheckCombatTarget(); // oyuncu
	}
	else
	{
		CheckPatrolTarget(); // devriye noktaları
	}
}


