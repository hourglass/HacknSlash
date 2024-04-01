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
	void DropWeaponByHit(class AWeapon*& Weapon);

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
	TSubclassOf<UCameraShakeBase> HitShake; // 카메라 쉐이크 클래스

	UPROPERTY()
	UParticleSystemComponent* HitParticle; // 공격 시 파티클

	class AWeapon* ManagedWeapon; // 현재 장착되어 관리될 무기

	ACharacter* OwnCharacter;	// 무기를 장착한 캐릭터 오브젝트

	ETraceTypeQuery AttackChannel; // 공격 충돌 체크 채널 
	TArray<AActor*> HitIgnore;	   // 충돌을 무시할 채널
	FVector PrevStartLoc;		   // 무기 소켓 이전 프레임 저장 변수
	FVector PrevEndLoc;			   // 무기 소켓 이전 프레임 저장 변수
	bool bFirstTake;			   // 공격 체크 시 첫번째 수행인지 확인하는 변수

	float EquipRadius;
	float FindRadius;
};
