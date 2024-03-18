// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "EnemyAnimInstance.h"
#include "EnemyAIController.h"
#include "WeaponManagerComponent.h"
#include "Weapon.h"
#include "HitReactionComponent.h"
#include "NavigationSystem.h"
#include "EnemyHealthBarWidget.h"
#include "Components/WidgetComponent.h"

// Sets default values
AEnemy::AEnemy()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Variable Setting //
	MaxCombo = 2;
	CurrentCombo = 0;
	MaxHealth = 400.f;
	CurrentHealth = MaxHealth;

	// Component Setting //
	// Mesh and Anim
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>
		SK_Body(TEXT("/Game/InfinityBladeWarriors/Character/CompleteCharacters/SK_CharM_FrostGiant.SK_CharM_FrostGiant"));

	if (SK_Body.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(SK_Body.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterial>
		M_BodyMatarial(TEXT("/Game/InfinityBladeWarriors/Character/CompleteCharacters/Textures_Materials/CharM_FrostGiant/M_Char_FrostGiant_Dissovle.M_Char_FrostGiant_Dissovle"));

	if (M_BodyMatarial.Succeeded())
	{
		GetMesh()->SetMaterial(0, M_BodyMatarial.Object);
	}

	static ConstructorHelpers::FClassFinder<UAnimInstance>
		ABP_Character(TEXT("/Game/Animations/ABP/ABP_Enemy.ABP_Enemy_C"));

	if (ABP_Character.Succeeded())
	{
		GetMesh()->SetAnimClass(ABP_Character.Class);
	}

	static ConstructorHelpers::FClassFinder<UUserWidget> UMG_HealthBarWidget(TEXT("/Game/UI/UMG_EnemyHealthBar.UMG_EnemyHealthBar_C"));
	HealthBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBar"));

	if (UMG_HealthBarWidget.Succeeded())
	{
		HealthBarWidget->SetWidgetClass(UMG_HealthBarWidget.Class);
		HealthBarWidget->SetDrawSize(FVector2D(100.f, 15.f));
	}

	// Mesh
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	GetMesh()->SetRelativeLocationAndRotation(
		FVector(0.f, 0.f, -88.f),
		FRotator(0.f, -90.f, 0.f)
	);

	// CapsuleComponent
	RootComponent = GetCapsuleComponent();
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Enemy"));

	// CharacterMovementComponent
	GetCharacterMovement()->bUseRVOAvoidance = true;

	// AIController
	AIControllerClass = AEnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// WeaponManagerComponent
	WeaponManager = CreateDefaultSubobject<UWeaponManagerComponent>(TEXT("WeaponManagerComponent"));

	// HitReactionComponent
	HitReactionComponent = CreateDefaultSubobject<UHitReactionComponent>(TEXT("HitReactionComponent"));

	// HealthBarWidget
	HealthBarWidget->SetupAttachment(RootComponent);
	HealthBarWidget->SetRelativeLocation(FVector(0.f, 0.f, 110.f));
	HealthBarWidget->SetWidgetSpace(EWidgetSpace::Screen);

	// Timeline Setting //
	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimeline"));

	static ConstructorHelpers::FObjectFinder<UCurveFloat>
		Curve_DissovleBorn(TEXT("/Game/Curves/Curve_DissolveBorn.Curve_DissolveBorn"));

	if (Curve_DissovleBorn.Succeeded())
	{
		DissolveCurve_Born = Curve_DissovleBorn.Object;
	}

	static ConstructorHelpers::FObjectFinder<UCurveFloat>
		Curve_DissovleDead(TEXT("/Game/Curves/Curve_DissolveDead.Curve_DissolveDead"));

	if (Curve_DissovleDead.Succeeded())
	{
		DissolveCurve_Dead = Curve_DissovleDead.Object;
	}
}

void AEnemy::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// EnemyAIController Setting // 
	EnemyController = Cast<AEnemyAIController>(GetController());

	// AnimInstance Setting //
	EnemyAnim = Cast<UEnemyAnimInstance>((GetMesh()->GetAnimInstance()));
	if (IsValid(EnemyAnim))
	{
		EnemyAnim->OnMontageEnded.AddDynamic(this, &AEnemy::OnActionMontageEnded);

		// Binding SendSwing Event
		EnemyAnim->OnSendSwing.AddLambda([this]()->void {
			if (IsValid(CurrentWeapon))
			{
				PlayAttackSlowMotion();
				CurrentWeapon->SetCurrentAttackType(EAttackType::Swing);
			}
			});

		// Binding SendSmash Event
		EnemyAnim->OnSendSmash.AddLambda([this]()->void {
			if (IsValid(CurrentWeapon))
			{
				PlayAttackSlowMotion();
				CurrentWeapon->SetCurrentAttackType(EAttackType::Smash);
			}
			});

		// Binding ComboCheck Event
		EnemyAnim->OnComboCheck.AddLambda([this]()->void {

			OnComboEnded.Broadcast();
			CurrentCombo = FMath::Clamp<int32>(CurrentCombo + 1, 0, MaxCombo);
			if (CurrentCombo >= MaxCombo)
			{
				EnemyAnim->StopAllMontages(0.2f);
			}
			});

		// Binding AttackStart Event
		EnemyAnim->OnAttackStart.AddLambda([this]()->void {
			AttackStart();
			});

		// Biding Change MoveDirection Event
		EnemyAnim->OnChangeMoveDirection.AddLambda([this]()->void {
			ChangeMoveSpeed();
			});
	}

	// Binding TimeLine Event
	DissolveFunction.BindUFunction(this, FName("DissolveByCurve"));

	// Binding HealthBarWidget
	HealthBarWidget->InitWidget();
	auto HealthBar = Cast<UEnemyHealthBarWidget>(HealthBarWidget->GetUserWidgetObject());
	if (HealthBar)
	{
		HealthBar->BindEnemyHealth(this);
	}
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	ChangeState(EnemyState::Spawned);
	BattleCry();
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

float AEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	ChangeState(EnemyState::Hit);
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("EnemyHitted"));

	EnemyController->ClearTargetActor();
	EnemyAnim->PlayHitMontage();

	HitReactionComponent->PlayHitImpact(DamageCauser);
	HitReactionComponent->PlayHitStun();
	HitReactionComponent->PlayRimLighting();

	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0, MaxHealth);
	OnHealthChanged.Broadcast();

	if (CurrentHealth <= 0)
	{
		Die();
	}

	return 0.f;
}

void AEnemy::SetCurrentWeapon(class AWeapon* Weapon)
{
	CurrentWeapon = Weapon;
	if (IsValid(CurrentWeapon))
	{
		CurrentWeapon->SetCanEquip(false);
		EnemyAnim->SetAttackMontage(CurrentWeapon->GetWeaponData()->AttackMontage);
		EnemyController->SetCurrentWeapon(CurrentWeapon);
		WeaponManager->SetManagedWeapon(CurrentWeapon);

		auto AttackChannel = UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel2);
		WeaponManager->SetAttackChannel(AttackChannel);

		MaxCombo = CurrentWeapon->GetMaxCombo();
	}
}

void AEnemy::Equip()
{
	WeaponManager->EquipWeapon(CurrentWeapon);
	if (IsValid(CurrentWeapon))
	{
		SetCurrentWeapon(CurrentWeapon);
		ChaseTarget();

		EnemyController->ClearTargetWeapon();
	}
	else
	{
		EnemyController->ClearCurrentWeapon();
	}
}

void AEnemy::Drop()
{
	WeaponManager->HittedDropWeapon(CurrentWeapon);
	EnemyController->ClearCurrentWeapon();
}

void AEnemy::Die()
{
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Ghost"));
	GetCharacterMovement()->StopMovementImmediately();
	
	EnemyAnim->SetIsDead(true);
	EnemyAnim->StopAllMontages(0.f);
	EnemyController->UnPossess();
	WeaponManager->DropWeapon(CurrentWeapon);

	HealthBarWidget->SetVisibility(false, false);

	DissolveTimeline->AddInterpFloat(DissolveCurve_Dead, DissolveFunction);
	DissolveTimeline->SetLooping(false);
	DissolveTimeline->PlayFromStart();

	FTimerHandle WaitHandle;
	float Waittime = 3.f;
	GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]() {

		Destroy();
		}),
		Waittime, false);
}

float AEnemy::GetHealthRatio()
{
	return CurrentHealth / MaxHealth;
}


void AEnemy::Attack()
{
	if (IsValid(CurrentWeapon))
	{
		ChangeState(EnemyState::Attack);
		EnemyAnim->PlayAttackMontage(CurrentWeapon->GetWeaponData()->AttackSpeed);
	}
}

void AEnemy::AttackStart()
{
	OnComboStart.Broadcast();
	WeaponManager->WeaponTrailBegin();

	if (IsValid(CurrentWeapon))
	{
		auto LaunchForce = (GetActorForwardVector() * CurrentWeapon->GetWeaponData()->LaunchForce);
		GetCharacterMovement()->StopMovementImmediately();
		LaunchCharacter(LaunchForce, false, false);
	}
}

void AEnemy::BattleCry()
{
	// Add 'One float Param Fucntion' for Delegate(FOnTimelineFloat Type)
	DissolveTimeline->AddInterpFloat(DissolveCurve_Born, DissolveFunction);
	DissolveTimeline->SetLooping(false);

	DissolveTimeline->PlayFromStart();
	EnemyAnim->PlayBattleCryMontage();
}

void AEnemy::DissolveByCurve(float Alpha)
{
	GetMesh()->SetScalarParameterValueOnMaterials(FName("DissolveIntensity"), Alpha);
}

void AEnemy::ChaseTarget()
{
	ChangeState(EnemyState::Chase);

	CurrentTarget = Cast<APawn>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	EnemyController->SetTargetActor(CurrentTarget);
}

void AEnemy::ChangeMoveSpeed()
{
	if (EnemyAnim->GetMoveBackward())
	{
		GetCharacterMovement()->MaxWalkSpeed = 150.f;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = 400.f;
	}
}

void AEnemy::PlayAttackSlowMotion()
{
	if (IsValid(CurrentWeapon))
	{
		GetMesh()->GetAnimInstance()->Montage_SetPlayRate(CurrentWeapon->GetWeaponData()->AttackMontage, 0.2f);
	}

	float WaitTime = 0.35f;
	FTimerHandle WaitHandle;
	GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]() {
		if (IsValid(CurrentWeapon))
		{
			GetMesh()->GetAnimInstance()->Montage_SetPlayRate(
				CurrentWeapon->GetWeaponData()->AttackMontage,
				CurrentWeapon->GetWeaponData()->AttackSpeed
			);
		}
		}), WaitTime, false);
}

void AEnemy::ChangeState(EnemyState State)
{
	PrevState = CurrentState;
	CurrentState = State;
	EnemyController->SetState((uint8)CurrentState);

	if (PrevState == EnemyState::Attack)
	{
		WeaponManager->WeaponTrailEnd();
	}

	if (CurrentState != EnemyState::Chase)
	{
		GetCharacterMovement()->bUseRVOAvoidance = false;
	}

	switch (CurrentState)
	{
	case EnemyState::Spawned:
		GetCapsuleComponent()->SetCollisionProfileName(TEXT("Ghost"));
		break;
	case EnemyState::Idle:
		GetCapsuleComponent()->SetCollisionProfileName(TEXT("Enemy"));
		break;
	case EnemyState::Chase:
		GetCharacterMovement()->bUseRVOAvoidance = true;
		break;
	case EnemyState::Attack:
		break;
	case EnemyState::Hit:
		break;
	default:
		break;
	}
}


// MontageEnded Fuction
void AEnemy::OnActionMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (CurrentState == EnemyState::Attack)
	{
		OnAttackEnded.Broadcast();
		ChangeState(EnemyState::Idle);
		ChaseTarget();
		CurrentCombo = 0;
	}

	if (CurrentState == EnemyState::Spawned)
	{
		ChangeState(EnemyState::Idle);
		ChaseTarget();
	}

	if (CurrentState == EnemyState::Hit && !bInterrupted)
	{
		GetCapsuleComponent()->SetCollisionProfileName(TEXT("Enemy"));
		if(CurrentWeapon)
		{
			ChaseTarget();
		}
		else
		{
			ChangeState(EnemyState::Idle);
			EnemyController->ClearTargetActor();
		}
	}
}


// Called By AttackableInterface
void AEnemy::AttackCheck()
{
	WeaponManager->AttackCheck();
}

void AEnemy::AttackCheckEnd()
{
	WeaponManager->AttackCheckEnd();
	WeaponManager->WeaponTrailEnd();
}

// Called By AIBehaviorInterface
AActor* AEnemy::GetActorFromBB(FName KeyName)
{
	return EnemyController->GetActorFromBB(KeyName);
}

void AEnemy::SetDestination(FVector Destination)
{
	EnemyController->SetDestination(Destination);
}

void AEnemy::FindWeapon()
{
	WeaponManager->FindWeapon(TargetWeapon);
	if (IsValid(TargetWeapon))
	{
		EnemyController->SetTargetWeapon(TargetWeapon);
	}
	else
	{
		EnemyController->ClearTargetWeapon();
	}
}

void AEnemy::EquipWeapon()
{
	Equip();
}

void AEnemy::AttackTarget()
{
	Attack();
}

// Called By HitEventInterface
void AEnemy::HitFinalAttack()
{
	Drop();
}