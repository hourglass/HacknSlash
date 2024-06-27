// Fill out your copyright notice in the Description page of Project Settings.


#include "HnsCharacter.h"
#include "HnsCharacterAnimInstance.h"
#include "WeaponManagerComponent.h"
#include "Weapon.h"
#include "HitReactionComponent.h"
#include "HealthBarWidget.h"
#include "Components/WidgetComponent.h"

// Sets default values
AHnsCharacter::AHnsCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Variable Setting //
	CurrentState = CharacterState::Idle; // 캐릭터 상태 초기화
	bCanChangeDir = true;				 // 방향 전환 상태 확인 변수
	bIsComboInput = false;				 // 콤보 공격 추가 입력 확인 변수
	CurrentCombo = 0;					 //	현재 콤보
	RollSpeed = 400.f;					 // 구르기 속도
	MaxHealth = 800.f;					 // 최대 체력
	CurrentHealth = MaxHealth;			 // 현재 체력

	// Component Setting //
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));

	// Mesh and Anim
	static ConstructorHelpers::FObjectFinder<USkeletalMesh>
		SK_Body(TEXT("/Game/InfinityBladeWarriors/Character/CompleteCharacters/SK_CharM_Barbarous.SK_CharM_Barbarous"));

	if (SK_Body.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(SK_Body.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterial>
		M_BodyMatarial(TEXT("/Game/InfinityBladeWarriors/Character/CompleteCharacters/Textures_Materials/CharM_Barbarous/M_Char_Barbrous_Inst.M_Char_Barbrous_Inst"));

	if (M_BodyMatarial.Succeeded())
	{
		GetMesh()->SetMaterial(0, M_BodyMatarial.Object);
	}

	static ConstructorHelpers::FClassFinder<UAnimInstance>
		ABP_Character(TEXT("/Game/Animations/ABP/ABP_Character.ABP_Character_C"));

	if (ABP_Character.Succeeded())
	{
		GetMesh()->SetAnimClass(ABP_Character.Class);
	}

	static ConstructorHelpers::FClassFinder<UUserWidget> UMG_HealthBarWidget(TEXT("/Game/UI/UMG_HealthBar.UMG_HealthBar_C"));
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

	// SpringArm and Camera
	SpringArm->TargetArmLength = 750.f;
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->SetupAttachment(GetCapsuleComponent());
	Camera->SetupAttachment(SpringArm);

	// CharacterMovement
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 1000.f, 0.f);

	// CapsuleComponent
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Character"));

	// WeaponManager
	WeaponManager = CreateDefaultSubobject<UWeaponManagerComponent>(TEXT("WeaponManager"));

	//HitReactionComponent
	HitReactionComponent = CreateDefaultSubobject<UHitReactionComponent>(TEXT("HitReactionComponent"));

	// HealthBarWidget
	HealthBarWidget->SetupAttachment(RootComponent);
	HealthBarWidget->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
	HealthBarWidget->SetWidgetSpace(EWidgetSpace::Screen);

	// Timeline Setting //
	RollTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("RollTimeline"));

	static ConstructorHelpers::FObjectFinder<UCurveFloat>
		Curve_Roll(TEXT("/Game/Curves/Curve_Roll.Curve_Roll"));

	if (Curve_Roll.Succeeded())
	{
		RollCurve = Curve_Roll.Object;
	}

	// Binding HealthBarWidget
	HealthBarWidget->InitWidget();
	auto HealthBar = Cast<UHealthBarWidget>(HealthBarWidget->GetUserWidgetObject());
	if (HealthBar)
	{
		HealthBar->BindCharacterHealth(this);
	}
}

// 컴포넌트 초기화 후 호출되는 함수
void  AHnsCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	ChangeState(CharacterState::Idle);

	// AnimInstance Setting //
	HnsCharacterAnim = Cast<UHnsCharacterAnimInstance>((GetMesh()->GetAnimInstance()));
	if (IsValid(HnsCharacterAnim))
	{
		// Binding MontageEnded Event
		// 몽타주 종료 후 호출되는 이벤트
		HnsCharacterAnim->OnMontageEnded.AddDynamic(this, &AHnsCharacter::OnActionMontageEnded);

		// Binding MontageBlendingOut Event
		// 몽타주 블렌드 아웃 시 호출되는 이벤트
		HnsCharacterAnim->OnMontageBlendingOut.AddDynamic(this, &AHnsCharacter::OnActionMontageBlendOut);

		// Binding SendSwing Event
		// 해당 몽타주의 공격 타입을 넘겨 주는 함수
		HnsCharacterAnim->OnSendSwing.AddLambda([this]()->void {
			if (IsValid(CurrentWeapon))
			{
				CurrentWeapon->SetCurrentAttackType(EAttackType::Swing);
			}
			});

		// Binding SendSmash Event
		// 해당 몽타주의 공격 타입을 넘겨 주는 함수
		HnsCharacterAnim->OnSendSmash.AddLambda([this]()->void {
			if (IsValid(CurrentWeapon))
			{
				CurrentWeapon->SetCurrentAttackType(EAttackType::Smash);
			}
			});

		// Binding ComboAttack Event
		// 공격 추가 입력 시 콤보 어택 이벤트 등록 함수
		HnsCharacterAnim->OnComboCheck.AddLambda([this]()->void {
			if (bIsComboInput)
			{
				bIsComboInput = false;
				bCanChangeDir = true;
				AttackComboStart();
			}
			else
			{
				ChangeState(CharacterState::Idle);
			}
			});

		// Binding AttackStart Event
		// 공격 시작 시 이벤트
		HnsCharacterAnim->OnAttackStart.AddLambda([this]()->void {
			AttackStart();
			});
	}

	// TimeLine Setting //
	if (IsValid(RollCurve))
	{
		// Binding Timeline Event
		// Add 'One float Param Fucntion' for Delegate(FOnTimelineFloat Type)
		// 구르기 타임라인 이벤트 등록
		RollFunction.BindUFunction(this, FName("RollByCurve"));
		RollTimeline->AddInterpFloat(RollCurve, RollFunction);
		RollTimeline->SetLooping(false);
	}
}

// Called when the game starts or when spawned
void AHnsCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Camera Rotation Setting
	if (IsValid(Controller))
	{
		Controller->SetControlRotation(FRotator(-60.f, 0.f, 0.f));
	}
}

// Called every frame
void AHnsCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


// Called to bind functionality to input
void AHnsCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// 키 입력 이벤트 등록
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Axis Event
	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AHnsCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AHnsCharacter::MoveRight);

	// Action Event
	PlayerInputComponent->BindAction(TEXT("Attack"), IE_Pressed, this, &AHnsCharacter::Attack);
	PlayerInputComponent->BindAction(TEXT("Dash"), IE_Pressed, this, &AHnsCharacter::Roll);
	PlayerInputComponent->BindAction(TEXT("Equip"), IE_Pressed, this, &AHnsCharacter::EquipAndDrop);
}

// 상하 이동 함수
void AHnsCharacter::MoveForward(float value)
{
	if (CurrentState != CharacterState::Idle)
	{
		return;
	}

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(Direction, value);
}

// 좌우 이동 함수
void AHnsCharacter::MoveRight(float value)
{
	if (CurrentState != CharacterState::Idle)
	{
		return;
	}

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(Direction, value);
}

// 마우스 클릭 방향으로 캐릭터를 회전하는 함수
void AHnsCharacter::LookToCursorDirection()
{
	if (bCanChangeDir)
	{
		bCanChangeDir = false;

		auto PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (IsValid(PlayerController))
		{
			float LocationX, LocationY;
			PlayerController->GetMousePosition(LocationX, LocationY);

			FVector2D MousePosition(LocationX, LocationY);

			FHitResult HitResult;
			if (PlayerController->GetHitResultAtScreenPosition(MousePosition, ECC_Visibility, true, HitResult))
			{
				FVector MouseDirection = HitResult.Location - GetActorLocation();

				FVector LookDirection(MouseDirection.X, MouseDirection.Y, 0.f);
				LookDirection.Normalize();

				GetCapsuleComponent()->SetRelativeRotation(LookDirection.Rotation());
			}
		}
	}
	else
	{
		return;
	}
}


// 공격 가능 상태 반환 함수
bool AHnsCharacter::CanAttack()
{
	bool Condition =
	{
		IsValid(CurrentWeapon) &&
		CurrentState != CharacterState::Roll &&
		CurrentState != CharacterState::Hit
	};

	return Condition;
}

// 공격 함수
void AHnsCharacter::Attack()
{
	// 공격이 가능한 상태인지 확인
	if (CanAttack())
	{
		// 마우스 클릭 방향으로 회전
		LookToCursorDirection();

		// 현재 상태 체크
		if (CurrentState != CharacterState::Attack)
		{
			// 캐릭터 상태 변경
			ChangeState(CharacterState::Attack);

			// 공격 시작
			AttackComboStart();
		}
		else
		{
			// 이미 공격 상태라면 콤보 상태로 전환
			bIsComboInput = true;
		}
	}
}

// Call By AnimNotify Event
// 공격 시작 함수
void AHnsCharacter::AttackStart()
{
	bCanChangeDir = false;

	// 트레일 재생
	WeaponManager->WeaponTrailBegin();
	
	// 무기가 있는 지 체크
	if (IsValid(CurrentWeapon))
	{
		// 공격 시 캐릭터를 앞으로 전진
		auto LaunchForce = (GetActorForwardVector() * CurrentWeapon->GetWeaponData()->LaunchForce);
		LaunchCharacter(LaunchForce, false, false);
	}
}

// 공격 추가 입력 시 처리 함수
void AHnsCharacter::AttackComboStart()
{
	// 현재 콤보 횟수 추가
	CurrentCombo = FMath::Clamp<int32>(CurrentCombo + 1, 1, CurrentWeapon->GetMaxCombo());

	// 다음 몽타주 섹션으로 이동
	HnsCharacterAnim->JumpToAttackMontageSection(CurrentCombo);

	// 공격 몽타주 재생
	HnsCharacterAnim->PlayAttackMontage(CurrentWeapon->GetWeaponData()->AttackSpeed);
}

// 공격 종료 처리 함수
void AHnsCharacter::AttackComboEnd()
{
	bIsComboInput = false;
	bCanChangeDir = true;
	CurrentCombo = 0;
}


// 구르기 가능 상태 반환 함수
bool AHnsCharacter::CanRoll()
{
	bool Condition =
	{
		CurrentState != CharacterState::Roll &&
		CurrentState != CharacterState::Hit
	};

	return Condition;
}

// 구르기 함수
void AHnsCharacter::Roll()
{
	// 구르기 가능 상태인지 확인
	if (CanRoll())
	{
		// 상태 전환
		ChangeState(CharacterState::Roll);

		// 마우스 클릭 방향으로 회전
		LookToCursorDirection();

		// 충돌 채널을 Ghost로 변경
		GetCapsuleComponent()->SetCollisionProfileName(TEXT("Ghost"));

		// 몽타주 재생
		HnsCharacterAnim->PlayRollMontage();

		// 구르기 타임라인 재생
		RollTimeline->PlayFromStart();
	}
}

// 구르기 타임라인 이벤트
void AHnsCharacter::RollByCurve(float DistanceMultiplier)
{
	// 전방 벡터 * 구르기 속도 * 커브 데이터 값으로 이동
	FVector DashDirection = GetActorForwardVector() * (RollSpeed * DistanceMultiplier);
	GetCharacterMovement()->Velocity.X = DashDirection.X;
	GetCharacterMovement()->Velocity.Y = DashDirection.Y;
}

// 상태 처리 함수
void AHnsCharacter::ChangeState(CharacterState State)
{
	// 공격 상태 처리
	if (CurrentState == CharacterState::Attack)
	{
		auto AttackMontage = CurrentWeapon->GetWeaponData()->AttackMontage;
		HnsCharacterAnim->Montage_Stop(0.2f, AttackMontage);
		WeaponManager->WeaponTrailEnd();
		AttackComboEnd();
	}

	// 구르기 상태 처리
	if (CurrentState == CharacterState::Roll)
	{
		bCanChangeDir = true;
		RollTimeline->Stop();
	}

	// 상태 갱신
	CurrentState = State;
}


// 무기 장착, 해제 함수
void AHnsCharacter::EquipAndDrop()
{
	// 공격 도중이라면 실행 불가
	if (CurrentState == CharacterState::Attack)
	{
		return;
	}

	if (!IsValid(CurrentWeapon))
	{
		// 무기가 없다면 장착 함수 수행
		WeaponManager->EquipWeapon(CurrentWeapon);
		if (IsValid(CurrentWeapon))
		{
			// 무기 정보로 부터 공격 몽타주 가져오기
			HnsCharacterAnim->SetAttackMontage(CurrentWeapon->GetWeaponData()->AttackMontage);

			// 공격 채널 설정
			auto AttackChannel = UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel1);
			WeaponManager->SetAttackChannel(AttackChannel);
		}
	}
	else
	{
		// 무기가 있다면 장착 해제 함수 수행
		WeaponManager->DropWeapon(CurrentWeapon);
	}
}


// MontageEnded Fuction
// 몽타주 종료 시 이벤트
void AHnsCharacter::OnActionMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// bInterrupted의 경우 몽타주가 완전히 종료 되지 않는 경우도 체크됨
	if (!bInterrupted)
	{
		// 공격 몽타주 종료 시 Idle로 변경
		if (CurrentState == CharacterState::Attack)
		{
			ChangeState(CharacterState::Idle);
		}
	}
}

// MontageBlendOut Fuction
// 몽타주 블렌드 아웃(몽타주 종료 후 원래 포즈로 돌아가는 구간) 이벤트
void AHnsCharacter::OnActionMontageBlendOut(UAnimMontage* Montage, bool bInterrupted)
{
	if (!bInterrupted)
	{
		if (CurrentState == CharacterState::Roll)
		{
			// 상태를 Idle로 변경
			ChangeState(CharacterState::Idle);

			// 충돌 채널을 Character로 복구
			GetCapsuleComponent()->SetCollisionProfileName(TEXT("Character"));

			// 구르기 모션이 끝난 후 타임라인 중지
			RollTimeline->Stop();
		}

		if (CurrentState == CharacterState::Hit)
		{
			// 피격 후 상태를 Idle로 변경
			ChangeState(CharacterState::Idle);
		}
	}
}


// Called By AttackableInterface
void AHnsCharacter::AttackCheck()
{
	// 공격 체크 함수 호출
	WeaponManager->AttackCheck();
}

void AHnsCharacter::AttackCheckEnd()
{
	// 공격 체크 종료 함수 호출
	WeaponManager->AttackCheckEnd();
	WeaponManager->WeaponTrailEnd();
}

bool AHnsCharacter::IsFinalAttack()
{
	// 마지막 콤보 공격인지 확인하는 함수
	if (IsValid(CurrentWeapon))
	{
		return CurrentCombo == CurrentWeapon->GetMaxCombo();
	}
	else
	{
		return false;
	}
}


// 피격 시 처리 함수
float AHnsCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	// 피격 몽타주 재생
	ChangeState(CharacterState::Hit);
	HnsCharacterAnim->PlayHitMontage();

	// 피격 시 타격감 함수 실행
	HitReactionComponent->PlayHitImpact(DamageCauser);
	HitReactionComponent->PlayHitStun();
	HitReactionComponent->PlayRimLighting();

	// 현재 체력 감소
	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0, MaxHealth);
	OnHealthChanged.Broadcast();

	// 사망 함수
	if (CurrentHealth <= 0)
	{
		Die();
	}

	return 0.f;
}

// 플레이어 사망 함수
void AHnsCharacter::Die()
{
	// 충돌 채널을 Ghost로 변경
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Ghost"));

	// 이동 중지
	GetCharacterMovement()->StopMovementImmediately();

	// 애님인스턴스을 죽음 상태로 변경
	HnsCharacterAnim->SetIsDead(true);

	// 몽타주 중지
	HnsCharacterAnim->StopAllMontages(0.f);

	// 무기 장착 해제
	WeaponManager->DropWeapon(CurrentWeapon);

	// HP 위젯 비활성화
	HealthBarWidget->SetVisibility(false, false);

	// 일정 시간 뒤 레벨 재시작
	FTimerHandle WaitHandle;
	float Waittime = 3.f;
	GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]() {

		UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()), false);
		}),
		Waittime, false);
}

float AHnsCharacter::GetHealthRatio()
{
	return CurrentHealth / MaxHealth;
}