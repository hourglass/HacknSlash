// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "HacknSlash.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

DECLARE_MULTICAST_DELEGATE(WeaponEventDelegate);

UENUM()
enum class EAttackType : int8
{
	Swing = 0 UMETA(DisplayName = "Swing"),
	Smash = 1 UMETA(DisplayName = "Smash"),
};

USTRUCT(BlueprintType)
struct FWeaponData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	FName WeaponName;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	USkeletalMesh* WeaponMesh;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	UAnimMontage* AttackMontage;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	TSubclassOf<UCameraShakeBase> HitShakeWeak;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	TSubclassOf<UCameraShakeBase> HitShakeStrong;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	float AttackDamage;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	float AttackSpeed;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	float AttackStiffness;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	float AttackRecovery;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	float LaunchForce;
};

UCLASS()
class HACKNSLASH_API AWeapon : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	void SetWeaponDataById(int Id);
	void SetWeaponDataByName(FName WeaponName);
	void SetCurrentAttackType(EAttackType NewType);
	void SetMaxComboBySection();
	void SetCanEquipByTimer();
	void SetCanEquip(bool State);

	struct FWeaponData* GetWeaponData();
	USkeletalMeshComponent* GetWeaponMesh();
	EAttackType GetCurrentAttackType();
	int32 GetMaxCombo();
	bool GetCanEquip();

	void WeaponTrailBegin();
	void WeaponTrailEnd();
	void PickUpEvent();

	WeaponEventDelegate OnPickUp;

private:
	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	USkeletalMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	UBoxComponent* WeaponCollision;

	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	UParticleSystemComponent* WeaponTrail;

	FWeaponData* WeaponData;

	EAttackType CurrentAttackType;

	int32 MaxCombo;

	bool bCanEquip;
};
