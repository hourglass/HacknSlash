// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "HacknSlash.h"
#include "AttackableInterface.h"
#include "AIBehaviorInterface.h"
#include "HitEventInterface.h"
#include "Components/TimeLineComponent.h"
#include "GameFramework/Character.h"
#include "Enemy.generated.h"


DECLARE_MULTICAST_DELEGATE(FOnHealthChanged);

UENUM(BlueprintType)
enum class EnemyState : uint8 {
	Spawned = 0 UMETA(DisplayName = "Spawned"),
	Idle = 1 UMETA(DisplayName = "Idle"),
	Chase = 2 UMETA(DisplayName = "Chase"),
	Attack = 3 UMETA(DisplayName = "Attack"),
	Hit = 4 UMETA(DisplayName = "Hit")
};

UCLASS()
class HACKNSLASH_API AEnemy : public ACharacter, public IAttackableInterface, public IAIBehaviorInterface, public IHitEventInterface
{
	GENERATED_BODY()
public:
	// Sets default values for this character's properties
	AEnemy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser);
	
	// Called By AttackableInterface
	virtual void AttackCheck() override;
	virtual void AttackCheckEnd() override;

	// Called By AIBehaviorInterface
	virtual AActor* GetActorFromBB(FName KeyName) override;
	virtual void SetDestination(FVector Destination) override;
	virtual void FindWeapon() override;
	virtual void EquipWeapon() override;
	virtual void AttackTarget() override;

	// Called By HitEventInterface
	virtual void HitFinalAttack() override;

	void SetCurrentWeapon(class AWeapon* Weapon);
	void Equip();
	void Drop();
	void Die();

	UFUNCTION()
	void OnActionMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void DissolveByCurve(float Alpha);

	UFUNCTION()
	float GetHealthRatio();

	UPROPERTY()
	class UEnemyAnimInstance* EnemyAnim;

	UPROPERTY()
	class AEnemyAIController* EnemyController;

	UPROPERTY()
	class UWeaponManagerComponent* WeaponManager;

	UPROPERTY()
	class UHitReactionComponent* HitReactionComponent;

	UPROPERTY(VisibleAnywhere, Category = UI)
	class UWidgetComponent* HealthBarWidget;

	FOnHealthChanged OnHealthChanged;

private:
	void Attack();
	void AttackStart();
	void BattleCry();
	void ChaseTarget();
	void ChangeMoveSpeed();
	void PlayAttackSlowMotion();
	void ChangeState(EnemyState State);

	UPROPERTY()
	class AWeapon* TargetWeapon;

	UPROPERTY()
	class AWeapon* CurrentWeapon;

	UPROPERTY()
	APawn* CurrentTarget;

	UPROPERTY()
	EnemyState CurrentState;

	UPROPERTY()
	EnemyState PrevState;

	UPROPERTY(BlueprintReadOnly, Category = "Timeline", Meta = (AllowPrivateAccess = true))
	UCurveFloat* DissolveCurve_Born;

	UPROPERTY(BlueprintReadOnly, Category = "Timeline", Meta = (AllowPrivateAccess = true))
	UCurveFloat* DissolveCurve_Dead;

	UPROPERTY()
	UTimelineComponent* DissolveTimeline;

	UPROPERTY()
	FOnTimelineFloat DissolveFunction;

	float MaxHealth;
	float CurrentHealth;

	int32 MaxCombo;
	int32 CurrentCombo;
};
