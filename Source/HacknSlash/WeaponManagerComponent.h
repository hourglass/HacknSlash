// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraShakeBase.h"
#include "Components/ActorComponent.h"
#include "WeaponManagerComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HACKNSLASH_API UWeaponManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UWeaponManagerComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void InitializeComponent() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SetManagedWeapon(class AWeapon* Weapon);
	void SetAttackChannel(ETraceTypeQuery NewAttackChannel);

	void EquipWeapon(class AWeapon*& Weapon);
	void FindWeapon(class AWeapon*& Weapon);
	void DropWeapon(class AWeapon*& Weapon);
	void HittedDropWeapon(class AWeapon*& Weapon);

	void AttackCheck();
	void AttackCheckSwing();
	void AttackCheckSmash();
	void AttackCheckEnd();

	void WeaponTrailBegin();
	void WeaponTrailEnd();

private:
	void GetNearestWeapon(class AWeapon*& Weapon, float SensingRadius);

	void PlayCameraShake();
	void PlayAttackStiffen();
	void LaunchTarget(AActor* HittedActor);
	void FinalAttackEvent(AActor* HittedActor);
	void PlayAttackReaction(AActor* HittedActor);

	UPROPERTY()
	TSubclassOf<UCameraShakeBase> HitShake;

	UPROPERTY()
	UParticleSystemComponent* HitParticle;

	class AWeapon* ManagedWeapon;

	ACharacter* OwnCharacter;

	ETraceTypeQuery AttackChannel;
	TArray<AActor*> HitIgnore;
	FVector PrevStartLoc;
	FVector PrevEndLoc;
	bool bFirstTake;

	float EquipRadius;
	float FindRadius;
};
