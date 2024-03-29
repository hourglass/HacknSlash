// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "HacknSlash.h"
#include "AttackableInterface.h"
#include "Components/TimeLineComponent.h"
#include "GameFramework/Character.h"
#include "HnsCharacter.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnHealthChanged);

UENUM()
enum class CharacterState : uint8 {
	Idle = 0 UMETA(DisplayName = "Idle"),
	Roll = 1 UMETA(DisplayName = "Roll"),
	Attack = 2 UMETA(DisplayName = "Attack"),
	Hit = 3 UMETA(DisplayName = "Hit")
};

UCLASS()
class HACKNSLASH_API AHnsCharacter : public ACharacter, public IAttackableInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AHnsCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void PostInitializeComponents() override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser);

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Called By Interface
	virtual void AttackCheck() override;
	virtual void AttackCheckEnd() override;
	virtual bool IsFinalAttack() override;

	UFUNCTION()
	float GetHealthRatio();

	UFUNCTION()
	void RollByCurve(float DistanceMultiplier);

	UFUNCTION()
	void OnActionMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnActionMontageBlendOut(UAnimMontage* Montage, bool bInterrupted);

	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* Camera;

	UPROPERTY()
	class UHnsCharacterAnimInstance* HnsCharacterAnim;

	UPROPERTY()
	class UWeaponManagerComponent* WeaponManager;

	UPROPERTY()
	class UHitReactionComponent* HitReactionComponent;

	UPROPERTY(VisibleAnywhere, Category = UI)
	class UWidgetComponent* HealthBarWidget;

	FOnHealthChanged OnHealthChanged;

private:
	void MoveForward(float value);
	void MoveRight(float value);
	void LookToCursorDirection();
	bool CanAttack();
	void Attack();
	void AttackStart();
	void AttackComboStart();
	void AttackComboEnd();
	bool CanRoll();
	void Roll();
	void EquipAndDrop();
	void Die();
	void ChangeState(CharacterState State);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timeline", Meta = (AllowPrivateAccess = true))
	UCurveFloat* RollCurve;

	UPROPERTY()
	UTimelineComponent* RollTimeline;

	UPROPERTY()
	FOnTimelineFloat RollFunction;

	UPROPERTY()
	class AWeapon* CurrentWeapon;

	UPROPERTY()
	CharacterState CurrentState;

	bool bCanChangeDir;
	bool bIsComboInput;
	int32 CurrentCombo;
	float RollSpeed;

	float MaxHealth;
	float CurrentHealth;
};
