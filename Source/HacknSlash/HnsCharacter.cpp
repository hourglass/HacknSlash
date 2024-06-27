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
	CurrentState = CharacterState::Idle; // ĳ���� ���� �ʱ�ȭ
	bCanChangeDir = true;				 // ���� ��ȯ ���� Ȯ�� ����
	bIsComboInput = false;				 // �޺� ���� �߰� �Է� Ȯ�� ����
	CurrentCombo = 0;					 //	���� �޺�
	RollSpeed = 400.f;					 // ������ �ӵ�
	MaxHealth = 800.f;					 // �ִ� ü��
	CurrentHealth = MaxHealth;			 // ���� ü��

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

// ������Ʈ �ʱ�ȭ �� ȣ��Ǵ� �Լ�
void  AHnsCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	ChangeState(CharacterState::Idle);

	// AnimInstance Setting //
	HnsCharacterAnim = Cast<UHnsCharacterAnimInstance>((GetMesh()->GetAnimInstance()));
	if (IsValid(HnsCharacterAnim))
	{
		// Binding MontageEnded Event
		// ��Ÿ�� ���� �� ȣ��Ǵ� �̺�Ʈ
		HnsCharacterAnim->OnMontageEnded.AddDynamic(this, &AHnsCharacter::OnActionMontageEnded);

		// Binding MontageBlendingOut Event
		// ��Ÿ�� ���� �ƿ� �� ȣ��Ǵ� �̺�Ʈ
		HnsCharacterAnim->OnMontageBlendingOut.AddDynamic(this, &AHnsCharacter::OnActionMontageBlendOut);

		// Binding SendSwing Event
		// �ش� ��Ÿ���� ���� Ÿ���� �Ѱ� �ִ� �Լ�
		HnsCharacterAnim->OnSendSwing.AddLambda([this]()->void {
			if (IsValid(CurrentWeapon))
			{
				CurrentWeapon->SetCurrentAttackType(EAttackType::Swing);
			}
			});

		// Binding SendSmash Event
		// �ش� ��Ÿ���� ���� Ÿ���� �Ѱ� �ִ� �Լ�
		HnsCharacterAnim->OnSendSmash.AddLambda([this]()->void {
			if (IsValid(CurrentWeapon))
			{
				CurrentWeapon->SetCurrentAttackType(EAttackType::Smash);
			}
			});

		// Binding ComboAttack Event
		// ���� �߰� �Է� �� �޺� ���� �̺�Ʈ ��� �Լ�
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
		// ���� ���� �� �̺�Ʈ
		HnsCharacterAnim->OnAttackStart.AddLambda([this]()->void {
			AttackStart();
			});
	}

	// TimeLine Setting //
	if (IsValid(RollCurve))
	{
		// Binding Timeline Event
		// Add 'One float Param Fucntion' for Delegate(FOnTimelineFloat Type)
		// ������ Ÿ�Ӷ��� �̺�Ʈ ���
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
	// Ű �Է� �̺�Ʈ ���
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Axis Event
	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AHnsCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AHnsCharacter::MoveRight);

	// Action Event
	PlayerInputComponent->BindAction(TEXT("Attack"), IE_Pressed, this, &AHnsCharacter::Attack);
	PlayerInputComponent->BindAction(TEXT("Dash"), IE_Pressed, this, &AHnsCharacter::Roll);
	PlayerInputComponent->BindAction(TEXT("Equip"), IE_Pressed, this, &AHnsCharacter::EquipAndDrop);
}

// ���� �̵� �Լ�
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

// �¿� �̵� �Լ�
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

// ���콺 Ŭ�� �������� ĳ���͸� ȸ���ϴ� �Լ�
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


// ���� ���� ���� ��ȯ �Լ�
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

// ���� �Լ�
void AHnsCharacter::Attack()
{
	// ������ ������ �������� Ȯ��
	if (CanAttack())
	{
		// ���콺 Ŭ�� �������� ȸ��
		LookToCursorDirection();

		// ���� ���� üũ
		if (CurrentState != CharacterState::Attack)
		{
			// ĳ���� ���� ����
			ChangeState(CharacterState::Attack);

			// ���� ����
			AttackComboStart();
		}
		else
		{
			// �̹� ���� ���¶�� �޺� ���·� ��ȯ
			bIsComboInput = true;
		}
	}
}

// Call By AnimNotify Event
// ���� ���� �Լ�
void AHnsCharacter::AttackStart()
{
	bCanChangeDir = false;

	// Ʈ���� ���
	WeaponManager->WeaponTrailBegin();
	
	// ���Ⱑ �ִ� �� üũ
	if (IsValid(CurrentWeapon))
	{
		// ���� �� ĳ���͸� ������ ����
		auto LaunchForce = (GetActorForwardVector() * CurrentWeapon->GetWeaponData()->LaunchForce);
		LaunchCharacter(LaunchForce, false, false);
	}
}

// ���� �߰� �Է� �� ó�� �Լ�
void AHnsCharacter::AttackComboStart()
{
	// ���� �޺� Ƚ�� �߰�
	CurrentCombo = FMath::Clamp<int32>(CurrentCombo + 1, 1, CurrentWeapon->GetMaxCombo());

	// ���� ��Ÿ�� �������� �̵�
	HnsCharacterAnim->JumpToAttackMontageSection(CurrentCombo);

	// ���� ��Ÿ�� ���
	HnsCharacterAnim->PlayAttackMontage(CurrentWeapon->GetWeaponData()->AttackSpeed);
}

// ���� ���� ó�� �Լ�
void AHnsCharacter::AttackComboEnd()
{
	bIsComboInput = false;
	bCanChangeDir = true;
	CurrentCombo = 0;
}


// ������ ���� ���� ��ȯ �Լ�
bool AHnsCharacter::CanRoll()
{
	bool Condition =
	{
		CurrentState != CharacterState::Roll &&
		CurrentState != CharacterState::Hit
	};

	return Condition;
}

// ������ �Լ�
void AHnsCharacter::Roll()
{
	// ������ ���� �������� Ȯ��
	if (CanRoll())
	{
		// ���� ��ȯ
		ChangeState(CharacterState::Roll);

		// ���콺 Ŭ�� �������� ȸ��
		LookToCursorDirection();

		// �浹 ä���� Ghost�� ����
		GetCapsuleComponent()->SetCollisionProfileName(TEXT("Ghost"));

		// ��Ÿ�� ���
		HnsCharacterAnim->PlayRollMontage();

		// ������ Ÿ�Ӷ��� ���
		RollTimeline->PlayFromStart();
	}
}

// ������ Ÿ�Ӷ��� �̺�Ʈ
void AHnsCharacter::RollByCurve(float DistanceMultiplier)
{
	// ���� ���� * ������ �ӵ� * Ŀ�� ������ ������ �̵�
	FVector DashDirection = GetActorForwardVector() * (RollSpeed * DistanceMultiplier);
	GetCharacterMovement()->Velocity.X = DashDirection.X;
	GetCharacterMovement()->Velocity.Y = DashDirection.Y;
}

// ���� ó�� �Լ�
void AHnsCharacter::ChangeState(CharacterState State)
{
	// ���� ���� ó��
	if (CurrentState == CharacterState::Attack)
	{
		auto AttackMontage = CurrentWeapon->GetWeaponData()->AttackMontage;
		HnsCharacterAnim->Montage_Stop(0.2f, AttackMontage);
		WeaponManager->WeaponTrailEnd();
		AttackComboEnd();
	}

	// ������ ���� ó��
	if (CurrentState == CharacterState::Roll)
	{
		bCanChangeDir = true;
		RollTimeline->Stop();
	}

	// ���� ����
	CurrentState = State;
}


// ���� ����, ���� �Լ�
void AHnsCharacter::EquipAndDrop()
{
	// ���� �����̶�� ���� �Ұ�
	if (CurrentState == CharacterState::Attack)
	{
		return;
	}

	if (!IsValid(CurrentWeapon))
	{
		// ���Ⱑ ���ٸ� ���� �Լ� ����
		WeaponManager->EquipWeapon(CurrentWeapon);
		if (IsValid(CurrentWeapon))
		{
			// ���� ������ ���� ���� ��Ÿ�� ��������
			HnsCharacterAnim->SetAttackMontage(CurrentWeapon->GetWeaponData()->AttackMontage);

			// ���� ä�� ����
			auto AttackChannel = UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel1);
			WeaponManager->SetAttackChannel(AttackChannel);
		}
	}
	else
	{
		// ���Ⱑ �ִٸ� ���� ���� �Լ� ����
		WeaponManager->DropWeapon(CurrentWeapon);
	}
}


// MontageEnded Fuction
// ��Ÿ�� ���� �� �̺�Ʈ
void AHnsCharacter::OnActionMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// bInterrupted�� ��� ��Ÿ�ְ� ������ ���� ���� �ʴ� ��쵵 üũ��
	if (!bInterrupted)
	{
		// ���� ��Ÿ�� ���� �� Idle�� ����
		if (CurrentState == CharacterState::Attack)
		{
			ChangeState(CharacterState::Idle);
		}
	}
}

// MontageBlendOut Fuction
// ��Ÿ�� ���� �ƿ�(��Ÿ�� ���� �� ���� ����� ���ư��� ����) �̺�Ʈ
void AHnsCharacter::OnActionMontageBlendOut(UAnimMontage* Montage, bool bInterrupted)
{
	if (!bInterrupted)
	{
		if (CurrentState == CharacterState::Roll)
		{
			// ���¸� Idle�� ����
			ChangeState(CharacterState::Idle);

			// �浹 ä���� Character�� ����
			GetCapsuleComponent()->SetCollisionProfileName(TEXT("Character"));

			// ������ ����� ���� �� Ÿ�Ӷ��� ����
			RollTimeline->Stop();
		}

		if (CurrentState == CharacterState::Hit)
		{
			// �ǰ� �� ���¸� Idle�� ����
			ChangeState(CharacterState::Idle);
		}
	}
}


// Called By AttackableInterface
void AHnsCharacter::AttackCheck()
{
	// ���� üũ �Լ� ȣ��
	WeaponManager->AttackCheck();
}

void AHnsCharacter::AttackCheckEnd()
{
	// ���� üũ ���� �Լ� ȣ��
	WeaponManager->AttackCheckEnd();
	WeaponManager->WeaponTrailEnd();
}

bool AHnsCharacter::IsFinalAttack()
{
	// ������ �޺� �������� Ȯ���ϴ� �Լ�
	if (IsValid(CurrentWeapon))
	{
		return CurrentCombo == CurrentWeapon->GetMaxCombo();
	}
	else
	{
		return false;
	}
}


// �ǰ� �� ó�� �Լ�
float AHnsCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	// �ǰ� ��Ÿ�� ���
	ChangeState(CharacterState::Hit);
	HnsCharacterAnim->PlayHitMontage();

	// �ǰ� �� Ÿ�ݰ� �Լ� ����
	HitReactionComponent->PlayHitImpact(DamageCauser);
	HitReactionComponent->PlayHitStun();
	HitReactionComponent->PlayRimLighting();

	// ���� ü�� ����
	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0, MaxHealth);
	OnHealthChanged.Broadcast();

	// ��� �Լ�
	if (CurrentHealth <= 0)
	{
		Die();
	}

	return 0.f;
}

// �÷��̾� ��� �Լ�
void AHnsCharacter::Die()
{
	// �浹 ä���� Ghost�� ����
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Ghost"));

	// �̵� ����
	GetCharacterMovement()->StopMovementImmediately();

	// �ִ��ν��Ͻ��� ���� ���·� ����
	HnsCharacterAnim->SetIsDead(true);

	// ��Ÿ�� ����
	HnsCharacterAnim->StopAllMontages(0.f);

	// ���� ���� ����
	WeaponManager->DropWeapon(CurrentWeapon);

	// HP ���� ��Ȱ��ȭ
	HealthBarWidget->SetVisibility(false, false);

	// ���� �ð� �� ���� �����
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