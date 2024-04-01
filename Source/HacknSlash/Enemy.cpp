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
		M_BodyMatarial(TEXT("/Game/InfinityBladeWarriors/Character/CompleteCharacters/Textures_Materials/CharM_FrostGiant/M_Char_FrostGiant_Inst.M_Char_FrostGiant_Inst"));

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

// 현재 무기 세팅 함수
void AEnemy::SetCurrentWeapon(class AWeapon* Weapon)
{
	CurrentWeapon = Weapon;
	if (IsValid(CurrentWeapon))
	{
		// 장착 불가 상태로 변경
		CurrentWeapon->SetCanEquip(false);

		// 공격 몽타주 세팅
		EnemyAnim->SetAttackMontage(CurrentWeapon->GetWeaponData()->AttackMontage);

		// 현재 무기로 등록
		EnemyController->SetCurrentWeapon(CurrentWeapon);
		WeaponManager->SetManagedWeapon(CurrentWeapon);

		// 공격 채널 세팅
		auto AttackChannel = UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel2);
		WeaponManager->SetAttackChannel(AttackChannel);

		// 최대 콤보 횟수 설정
		MaxCombo = CurrentWeapon->GetMaxCombo();
	}
}

// 무기 장착 함수
void AEnemy::Equip()
{
	// 무기 탐색 후 장착
	WeaponManager->EquipWeapon(CurrentWeapon);
	if (IsValid(CurrentWeapon))
	{
		// 탐색한 무기를 현재 무기로 세팅
		SetCurrentWeapon(CurrentWeapon);

		// 플레이어 추적
		ChaseTarget();
		 
		// Behavior Tree 갱신
		EnemyController->ClearTargetWeapon();
	}
	else
	{
		// Behavior Tree 갱신
		EnemyController->ClearCurrentWeapon();
	}
}

// 무기 장착 해제 함수
void AEnemy::Drop()
{
	// 피격 시 무기 해체 함수 호출
	WeaponManager->DropWeaponByHit(CurrentWeapon);

	// Behavior Tree 갱신
	EnemyController->ClearCurrentWeapon();
}


// 공격 함수
void AEnemy::Attack()
{
	if (IsValid(CurrentWeapon))
	{
		// 상태 전환
		ChangeState(EnemyState::Attack);

		// 몽타주 재생
		EnemyAnim->PlayAttackMontage(CurrentWeapon->GetWeaponData()->AttackSpeed);
	}
}

// 공격 시작 함수
void AEnemy::AttackStart()
{
	// UAIBehaviorInterface 델리게이트
	OnComboStart.Broadcast();

	// 트레일 재생
	WeaponManager->WeaponTrailBegin();

	if (IsValid(CurrentWeapon))
	{
		// 공격 시 캐릭터 전진
		auto LaunchForce = (GetActorForwardVector() * CurrentWeapon->GetWeaponData()->LaunchForce);
		GetCharacterMovement()->StopMovementImmediately();
		LaunchCharacter(LaunchForce, false, false);
	}
}

void AEnemy::PlayAttackSlowMotion()
{
	// 밸런스를 위해 적군의 공격 속도 조절
	if (IsValid(CurrentWeapon))
	{
		GetMesh()->GetAnimInstance()->Montage_SetPlayRate(CurrentWeapon->GetWeaponData()->AttackMontage, 0.2f);
	}

	// 공격 시간 동안만 몽타주 속도 감소
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


// 피해 처리 함수
float AEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	// 상태 전환
	ChangeState(EnemyState::Hit);

	// 충돌 채널을 EnemyHitted로 변경
	// 공격으로 밀려나면서 적끼리 충돌 하는것을 방지
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("EnemyHitted"));

	// Behavior Tree 갱신
	EnemyController->ClearTargetActor();
	EnemyAnim->PlayHitMontage();
	
	// 타격감 함수 수행
	HitReactionComponent->PlayHitImpact(DamageCauser);
	HitReactionComponent->PlayHitStun();
	HitReactionComponent->PlayRimLighting();

	// 체력 감소
	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0, MaxHealth);
	OnHealthChanged.Broadcast();

	// 사망 함수
	if (CurrentHealth <= 0)
	{
		Die();
	}

	return 0.f;
}

void AEnemy::Die()
{
	// 충돌 채널을 Ghost로 변경
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Ghost"));

	// 이동 중지
	GetCharacterMovement()->StopMovementImmediately();

	// 애님인스턴스 상태를 죽음으로 변경
	EnemyAnim->SetIsDead(true);

	// 몽타주 중지
	EnemyAnim->StopAllMontages(0.f);
	
	// AIController 중지
	EnemyController->UnPossess();

	// 무기 장착 해제
	WeaponManager->DropWeapon(CurrentWeapon);

	// Hp바 위젯 비활성화
	HealthBarWidget->SetVisibility(false, false);

	// 디졸브 타임라인 재생
	DissolveTimeline->AddInterpFloat(DissolveCurve_Dead, DissolveFunction);
	DissolveTimeline->SetLooping(false);
	DissolveTimeline->PlayFromStart();

	// 파괴 함수 호출
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


// 생성 시 디졸브 효과과 몽타주 실행 함수
void AEnemy::BattleCry()
{
	// Add 'One float Param Fucntion' for Delegate(FOnTimelineFloat Type)
	DissolveTimeline->AddInterpFloat(DissolveCurve_Born, DissolveFunction);
	DissolveTimeline->SetLooping(false);

	DissolveTimeline->PlayFromStart();
	EnemyAnim->PlayBattleCryMontage();
}

// 디졸브 효과 타임라인
void AEnemy::DissolveByCurve(float Alpha)
{
	GetMesh()->SetScalarParameterValueOnMaterials(FName("DissolveIntensity"), Alpha);
}


// 플레이어 추적 함수
void AEnemy::ChaseTarget()
{
	// 상태 전환
	ChangeState(EnemyState::Chase);

	// 추적 상대 설정
	CurrentTarget = Cast<APawn>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	// Behavior Tree 갱신
	EnemyController->SetTargetActor(CurrentTarget);
}

// 추적 상태에 따른 이동 속도 변경 함수
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

// 상태 전환 처리 함수
void AEnemy::ChangeState(EnemyState State)
{
	PrevState = CurrentState;
	CurrentState = State;
	EnemyController->SetState((uint8)CurrentState);

	if (PrevState == EnemyState::Attack)
	{
		// 이전 상태가 공격이었다면 트레일 중지
		WeaponManager->WeaponTrailEnd();
	}

	if (CurrentState != EnemyState::Chase)
	{
		// 플레이어 추적 상태가 아닐 때는 서로의 경로를 계산하지 않도록 설정
		GetCharacterMovement()->bUseRVOAvoidance = false;
	}

	// 상태에 따라 맞는 함수 처리
	switch (CurrentState)
	{
	case EnemyState::Spawned:
		// 소환 시 충돌 채널을 Ghost로 설정
		GetCapsuleComponent()->SetCollisionProfileName(TEXT("Ghost"));
		break;
	case EnemyState::Idle:
		// 소환 후 충돌 채널을 Enemy로 설정
		GetCapsuleComponent()->SetCollisionProfileName(TEXT("Enemy"));
		break;
	case EnemyState::Chase:
		// 플레이어 추적 시 서로의 경로를 계산하도록 설정
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
// 몽타주 종료 시 이벤트
void AEnemy::OnActionMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (CurrentState == EnemyState::Attack)
	{
		// UAIBehaviorInterface 공격 종료 시 이벤트
		OnAttackEnded.Broadcast();

		// 상태 전환
		ChangeState(EnemyState::Idle);

		// 플레이어 추적
		ChaseTarget();

		// 콤보 초기화
		CurrentCombo = 0;
	}

	if (CurrentState == EnemyState::Spawned)
	{
		// 소환 완료 후 상태를 Idle로 변경
		ChangeState(EnemyState::Idle);
		ChaseTarget();
	}

	if (CurrentState == EnemyState::Hit && !bInterrupted)
	{
		// 피격 종료 후 충돌 채널을 Enemy로 변경
		GetCapsuleComponent()->SetCollisionProfileName(TEXT("Enemy"));
		if(CurrentWeapon)
		{
			//무기가 있다면 플레이어 추적
			ChaseTarget();
		}
		else
		{
			// 상태 전환
			ChangeState(EnemyState::Idle);

			// 무기가 없다면 무기 탐색
			EnemyController->ClearTargetActor();
		}
	}
}


// Called By AttackableInterface
void AEnemy::AttackCheck()
{
	// 공격 체크 함수 수행
	WeaponManager->AttackCheck();
}

void AEnemy::AttackCheckEnd()
{
	// 공격 체크 종료 함수 수행
	WeaponManager->AttackCheckEnd();
	WeaponManager->WeaponTrailEnd();
}

// Called By AIBehaviorInterface
// Behavior Tree의 블랙보드를 가져오는 함수
AActor* AEnemy::GetActorFromBB(FName KeyName)
{
	return EnemyController->GetActorFromBB(KeyName);
}

// AI 목적지 설정 함수
void AEnemy::SetDestination(FVector Destination)
{
	EnemyController->SetDestination(Destination);
}

// 무기 탐색 함수
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

// 무기 장착 함수
void AEnemy::EquipWeapon()
{
	Equip();
}

// AI 공격 처리 함수
void AEnemy::AttackTarget()
{
	Attack();
}

// Called By HitEventInterface
void AEnemy::HitFinalAttack()
{
	Drop();
}