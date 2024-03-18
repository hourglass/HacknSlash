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
	CurrentState = CharacterState::Idle;
	bCanChangeDir = true;
	bIsComboInput = false;
	CurrentCombo = 0;
	MaxDashCount = 2;
	CurrentDashCount = MaxDashCount;
	RollSpeed = 400.f;
	MaxHealth = 800.f;
	CurrentHealth = MaxHealth;

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
		M_BodyMatarial(TEXT("/Game/InfinityBladeWarriors/Character/CompleteCharacters/Textures_Materials/CharM_Barbarous/M_Char_Barbrous_Dissolve.M_Char_Barbrous_Dissolve"));

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

void  AHnsCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	ChangeState(CharacterState::Idle);

	// AnimInstance Setting //
	HnsCharacterAnim = Cast<UHnsCharacterAnimInstance>((GetMesh()->GetAnimInstance()));
	if (IsValid(HnsCharacterAnim))
	{
		// Binding MontageEnded Event
		HnsCharacterAnim->OnMontageEnded.AddDynamic(this, &AHnsCharacter::OnActionMontageEnded);

		// Binding MontageBlendingOut Event
		HnsCharacterAnim->OnMontageBlendingOut.AddDynamic(this, &AHnsCharacter::OnActionMontageBlendOut);

		// Binding SendSwing Event
		HnsCharacterAnim->OnSendSwing.AddLambda([this]()->void {
			if (IsValid(CurrentWeapon))
			{
				CurrentWeapon->SetCurrentAttackType(EAttackType::Swing);
			}
			});

		// Binding SendSmash Event
		HnsCharacterAnim->OnSendSmash.AddLambda([this]()->void {
			if (IsValid(CurrentWeapon))
			{
				CurrentWeapon->SetCurrentAttackType(EAttackType::Smash);
			}
			});

		// Binding ComboAttack Event
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
		HnsCharacterAnim->OnAttackStart.AddLambda([this]()->void {
			AttackStart();
			});
	}

	// TimeLine Setting //
	if (IsValid(RollCurve))
	{
		// Binding Timeline Event
		// Add 'One float Param Fucntion' for Delegate(FOnTimelineFloat Type)
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

float AHnsCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	ChangeState(CharacterState::Hit);
	HnsCharacterAnim->PlayHitMontage();

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


// Called to bind functionality to input
void AHnsCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Axis Event
	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AHnsCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AHnsCharacter::MoveRight);

	// Action Event
	PlayerInputComponent->BindAction(TEXT("Attack"), IE_Pressed, this, &AHnsCharacter::Attack);
	PlayerInputComponent->BindAction(TEXT("Dash"), IE_Pressed, this, &AHnsCharacter::Roll);
	PlayerInputComponent->BindAction(TEXT("Equip"), IE_Pressed, this, &AHnsCharacter::EquipAndDrop);
}

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

void AHnsCharacter::LookToCursorDirection()
{
	if (bCanChangeDir)
	{
		bCanChangeDir = false;

		FVector WorldLocation;
		FVector WorldDirection;
		FVector MouseLocation;

		auto PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (IsValid(PlayerController))
		{
			PlayerController->DeprojectMousePositionToWorld(WorldLocation, WorldDirection);

			MouseLocation = (WorldLocation.Z / WorldDirection.Z) * (WorldDirection * -1.f) + WorldLocation;
		}

		FVector MouseDirection = MouseLocation - GetActorLocation();
		FVector LookDirection(MouseDirection.X, MouseDirection.Y, 0.f);
		LookDirection.Normalize();

		GetCapsuleComponent()->SetRelativeRotation(LookDirection.Rotation());
	}
	else
	{
		return;
	}
}

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

void AHnsCharacter::Attack()
{
	if (CanAttack())
	{
		LookToCursorDirection();

		if (CurrentState == CharacterState::Attack)
		{
			bIsComboInput = true;
		}
		else
		{
			ChangeState(CharacterState::Attack);
			AttackComboStart();
		}
	}
}

// Call By AnimNotify Event
void AHnsCharacter::AttackStart()
{
	bCanChangeDir = false;
	WeaponManager->WeaponTrailBegin();

	if (IsValid(CurrentWeapon))
	{
		auto LaunchForce = (GetActorForwardVector() * CurrentWeapon->GetWeaponData()->LaunchForce);
		LaunchCharacter(LaunchForce, false, false);
	}
}
	
void AHnsCharacter::AttackComboStart()
{
	CurrentCombo = FMath::Clamp<int32>(CurrentCombo + 1, 1, CurrentWeapon->GetMaxCombo());
	HnsCharacterAnim->JumpToAttackMontageSection(CurrentCombo);
	HnsCharacterAnim->PlayAttackMontage(CurrentWeapon->GetWeaponData()->AttackSpeed);
}

void AHnsCharacter::AttackComboEnd()
{
	bIsComboInput = false;
	bCanChangeDir = true;
	CurrentCombo = 0;
}

bool AHnsCharacter::CanRoll()
{
	bool Condition =
	{
		CurrentState != CharacterState::Roll &&
		CurrentState != CharacterState::Hit
	};

	return Condition;
}

void AHnsCharacter::Roll()
{
	if (CanRoll())
	{
		ChangeState(CharacterState::Roll);

		LookToCursorDirection();
		GetCapsuleComponent()->SetCollisionProfileName(TEXT("Ghost"));
		HnsCharacterAnim->PlayRollMontage();
		RollTimeline->PlayFromStart();
	}
}

void AHnsCharacter::RollByCurve(float DistanceMultiplier)
{
	FVector DashDirection = GetActorForwardVector() * (RollSpeed * DistanceMultiplier);
	GetCharacterMovement()->Velocity.X = DashDirection.X;
	GetCharacterMovement()->Velocity.Y = DashDirection.Y;
}


void AHnsCharacter::EquipAndDrop()
{
	if (CurrentState == CharacterState::Attack)
	{
		return;
	}

	if (!IsValid(CurrentWeapon))
	{
		WeaponManager->EquipWeapon(CurrentWeapon);
		if (IsValid(CurrentWeapon))
		{
			HnsCharacterAnim->SetAttackMontage(CurrentWeapon->GetWeaponData()->AttackMontage);

			auto AttackChannel = UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel1);
			WeaponManager->SetAttackChannel(AttackChannel);
		}
	}
	else
	{
		WeaponManager->DropWeapon(CurrentWeapon);
	}
}

void AHnsCharacter::Die()
{
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Ghost"));
	GetCharacterMovement()->StopMovementImmediately();

	HnsCharacterAnim->SetIsDead(true);
	HnsCharacterAnim->StopAllMontages(0.f);
	WeaponManager->DropWeapon(CurrentWeapon);
	HealthBarWidget->SetVisibility(false, false);

	FTimerHandle WaitHandle;
	float Waittime = 3.f;
	GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]() {

		UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()), false);
		}),
		Waittime, false);
}

void AHnsCharacter::ChangeState(CharacterState State)
{
	PrevState = CurrentState;
	CurrentState = State;

	if (PrevState == CharacterState::Attack)
	{
		auto AttackMontage = CurrentWeapon->GetWeaponData()->AttackMontage;
		HnsCharacterAnim->Montage_Stop(0.2f, AttackMontage);
		WeaponManager->WeaponTrailEnd();
		AttackComboEnd();
	}

	if (PrevState == CharacterState::Roll)
	{
		bCanChangeDir = true;
		RollTimeline->Stop();
	}
}

float AHnsCharacter::GetHealthRatio()
{
	return CurrentHealth / MaxHealth;
}


// MontageEnded Fuction
void AHnsCharacter::OnActionMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!bInterrupted)
	{
		//TODO:::Add Decrease MoveCount
		if (CurrentState == CharacterState::Attack)
		{
			ChangeState(CharacterState::Idle);
		}
	}
}

// MontageBlendOut Fuction
void AHnsCharacter::OnActionMontageBlendOut(UAnimMontage* Montage, bool bInterrupted)
{
	if (!bInterrupted)
	{
		//TODO:::Add Decrease MoveCount
		if (CurrentState == CharacterState::Roll)
		{
			ChangeState(CharacterState::Idle);
			GetCapsuleComponent()->SetCollisionProfileName(TEXT("Character"));
			RollTimeline->Stop();
		}

		if (CurrentState == CharacterState::Hit)
		{
			ChangeState(CharacterState::Idle);
		}
	}
}


// Called By AttackableInterface
void AHnsCharacter::AttackCheck()
{
	WeaponManager->AttackCheck();
}

void AHnsCharacter::AttackCheckEnd()
{
	WeaponManager->AttackCheckEnd();
	WeaponManager->WeaponTrailEnd();
}

bool AHnsCharacter::IsFinalAttack()
{
	if (IsValid(CurrentWeapon))
	{
		return CurrentCombo == CurrentWeapon->GetMaxCombo();
	}
	else
	{
		return false;
	}
}