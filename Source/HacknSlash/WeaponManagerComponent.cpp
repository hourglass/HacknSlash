// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponManagerComponent.h"
#include "Weapon.h"
#include "AttackableInterface.h"
#include "HitEventInterface.h"

// Sets default values for this component's properties
UWeaponManagerComponent::UWeaponManagerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	bWantsInitializeComponent = true;

	// Variable Setting //
	bFirstTake = true;
	EquipRadius = 200.f;
	FindRadius = 2000.f;

	// Component Setting // 
	HitParticle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("HitParticle"));

	static ConstructorHelpers::FObjectFinder<UParticleSystem>
		HitEffect(TEXT("/Game/FXVarietyPack/Particles/P_ky_hit2.P_ky_hit2"));

	if (HitEffect.Succeeded())
	{
		HitParticle->SetTemplate(HitEffect.Object);
		HitParticle->SetAutoActivate(false);
	}
}

void UWeaponManagerComponent::InitializeComponent()
{
	// Character Setting //
	OwnCharacter = Cast<ACharacter>(GetOwner());
}

// Called when the game starts
void UWeaponManagerComponent::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void UWeaponManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}


void UWeaponManagerComponent::SetManagedWeapon(AWeapon* Weapon)
{
	ManagedWeapon = Weapon;
}

void UWeaponManagerComponent::SetAttackChannel(ETraceTypeQuery NewAttackChannel)
{
	AttackChannel = NewAttackChannel;
}

// 무기 장착 함수
void UWeaponManagerComponent::EquipWeapon(AWeapon*& Weapon)
{
	if (IsValid(Weapon))
	{
		return;
	}

	// 주울 수 있는 가장 가까운 무기 탐색
	GetNearestWeapon(Weapon, EquipRadius);

	// 무기 탐색이 성공했으면
	if (IsValid(Weapon))	
	{
		// 관리할 무기로 세팅
		SetManagedWeapon(Weapon);

		// 현재 캐릭터의 자식 객체로 설정
		if (IsValid(OwnCharacter))
		{
			// 모델의 손 위치에 있는 소켓에 위치하도록 설정
			Weapon->AttachToComponent(OwnCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, FName("Weapon"));
			Weapon->SetOwner(OwnCharacter);

			// 다른 캐릭터가 장착 무기를 줍지 못하도록 설정
			Weapon->SetCanEquip(false);

			// 무기를 주웠을 때 이벤트 실행
			Weapon->PickUpEvent();
		}
	}
}

// 장착한 무기를 떨어트리는 함수
void UWeaponManagerComponent::DropWeapon(AWeapon*& Weapon)
{ 
	if (IsValid(Weapon))
	{
		// 트랜스폼 자식 상태 해체
		Weapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform); 

		// 무기 장착 쿨타임 실행
		Weapon->SetCanEquipByTimer();

		// 무기 트레일 종료
		Weapon->WeaponTrailEnd();	

		// 떨어트린 무기의 트랜스폼 세팅
		FVector WeaponLoc = Weapon->GetActorLocation();
		Weapon->SetActorLocation(FVector(WeaponLoc.X, WeaponLoc.Y, 50.f));
		Weapon->SetActorRotation(FRotator::ZeroRotator);

		// 현재 무기 변수 초기화
		Weapon = nullptr;
		SetManagedWeapon(nullptr);
	}
}

// 피격 시 무기를 떨어트리는 함수
void UWeaponManagerComponent::DropWeaponByHit(AWeapon*& Weapon)
{
	if (IsValid(Weapon))
	{
		// 트랜스폼 자식 상태 해체
		Weapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		// 무기 장착 쿨타임 실행
		Weapon->SetCanEquipByTimer();

		// 무기 트레일 종료
		Weapon->WeaponTrailEnd();	  

		// 떨어트린 무기의 트랜스폼 세팅
		FVector OwnerForward = GetOwner()->GetActorForwardVector();
		FVector WeaponLoc = Weapon->GetActorLocation() - (OwnerForward * 250.f);

		Weapon->SetActorLocation(FVector(WeaponLoc.X, WeaponLoc.Y, 50.f));
		Weapon->SetActorRotation(FRotator::ZeroRotator);

		// 현재 무기 변수 초기화
		Weapon = nullptr;
		SetManagedWeapon(nullptr);
	}
}


// 가장 가까운 무기를 탐색하는 함수
void UWeaponManagerComponent::GetNearestWeapon(AWeapon*& Weapon, float SensingRadius)
{
	// 이 컴포넌트를 가진 캐릭터의 로케이션 가져오기
	FVector CharterLoc = GetOwner()->GetActorLocation();

	// 무기 탐색 충돌 체크 채널 설정
	ETraceTypeQuery WeaponSensingChannel =
		UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel3);

	// 충돌을 무시할 액터 설정
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Init(GetOwner(), 1);

	// 충돌 성공 시 정보를 가져올 배열
	TArray<FHitResult> HitArray;

	// SphereTraceMulti 충돌 체크
	bool Hit = UKismetSystemLibrary::SphereTraceMulti(GetWorld(), CharterLoc, CharterLoc, SensingRadius,
		WeaponSensingChannel, false, IgnoreActors, EDrawDebugTrace::None, HitArray, true);

	// 충돌 성공 시
	if (Hit)
	{
		// 탐색한 무기를 저장할 변수
		AWeapon* NearestWeapon = nullptr;

		// 기준이 될 최소값 설정
		float MinDistance = SensingRadius;

		// 배열을 순회하며 가장 가까운 무기를 탐색
		for (const auto& HitResult : HitArray)
		{
			auto DetectedWeapon = Cast<AWeapon>(HitResult.GetActor());
			if (IsValid(DetectedWeapon))
			{
				// 무기가 장착 불가능한 상태라면 다음 무기 탐색
				if (!DetectedWeapon->GetCanEquip())
				{
					continue;
				}

				// 무기와 캐릭터의 거리 계산후 최소값 갱신
				float Distance = DetectedWeapon->GetDistanceTo(GetOwner());
				if (Distance <= MinDistance)
				{
					NearestWeapon = DetectedWeapon;
					MinDistance = Distance;
				}
			}
		}

		// 가장 가까운 무기를 장착할 무기로 설정
		Weapon = NearestWeapon;
	}
	else
	{
		return;
	}
}


// 공격 시 타격감을 위한 함수 실행
void UWeaponManagerComponent::PlayAttackReaction(AActor* HittedActor)
{
	// Play Attack Reation
	PlayCameraShake();
	PlayAttackStiffen();
	LaunchTarget(HittedActor);
	FinalAttackEvent(HittedActor);
}

// 카메라 쉐이크 실행 함수
void UWeaponManagerComponent::PlayCameraShake()
{
	// 무기에 저장된 카메라 쉐이크 에셋 정보 가져오기
	HitShake = ManagedWeapon->GetWeaponData()->HitShakeWeak;
	if (IsValid(HitShake))
	{
		// 에셋으로 저장한 카메라 쉐이크 수행
		UGameplayStatics::PlayWorldCameraShake(GetWorld(), HitShake, GetOwner()->GetActorLocation(), 0, 1000, 1.f, false);
	}
}

// 공격 성공 시 애니메이션을 순간적으로 느리게 만드는 함수
void UWeaponManagerComponent::PlayAttackStiffen()
{
	if (IsValid(OwnCharacter) && IsValid(ManagedWeapon))
	{
		// 캐릭터의 애님 인스턴스에 접근해 몽타주 속도를 조절
		// 무기 정보에 있는 느려지는 수치를 적용
		OwnCharacter->GetMesh()->GetAnimInstance()->Montage_SetPlayRate(
			ManagedWeapon->GetWeaponData()->AttackMontage,
			ManagedWeapon->GetWeaponData()->AttackStiffness
		);

		// 무기 정보에 있는 기존 속도 복구 시간을 가져오기
		auto AttackRecovery = ManagedWeapon->GetWeaponData()->AttackRecovery;

		// 타이머를 통해 일정 시간후에 원래 속도로 복구
		FTimerHandle WaitHandle;
		GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]() {
			if (IsValid(OwnCharacter) && IsValid(ManagedWeapon))
			{
				OwnCharacter->GetMesh()->GetAnimInstance()->Montage_SetPlayRate(
					ManagedWeapon->GetWeaponData()->AttackMontage,
					ManagedWeapon->GetWeaponData()->AttackSpeed
				);
			}
			}), AttackRecovery, false);
	}
}

// 공격에 맞은 타겟을 뒤로 넉백시키는 함수
void UWeaponManagerComponent::LaunchTarget(AActor* HittedActor)
{
	if (IsValid(ManagedWeapon))
	{
		auto Target = Cast<ACharacter>(HittedActor);
		if (Target)
		{
			// 공격 대상을 향하는 벡터 구하기
			auto OwnerActor = GetOwner();
			auto TargetVector = Target->GetActorLocation() - OwnerActor->GetActorLocation();

			// 벡터의 내적으로 공격한 대상이 캐릭터의 뒤에 있는지 확인
			auto TargetDir = FVector::DotProduct(TargetVector, OwnerActor->GetActorForwardVector());

			// cos값이 0보다 크면 정면, 작으면 후방
			float Direction;
			(TargetDir >= 0) ? Direction = 1.f : Direction = -1.f;

			// 캐릭터 뒤 쪽의 적은 뒤로 밀려나도록 설정
			auto FowardVector = OwnerActor->GetActorForwardVector();

			// 무기에서 넉백 시킬 수치를 가져오기
			float LaunchForce = ManagedWeapon->GetWeaponData()->LaunchForce * Direction;

			// 넉백 실행
			Target->LaunchCharacter(FowardVector * LaunchForce, false, false);
		}
	}
}

// 마지막 콤보 시에 발생할 이벤트
void UWeaponManagerComponent::FinalAttackEvent(AActor* HittedActor)
{ 
	// 공격 가능한 물체가 상속하는 인터페이스 가져오기
	bool HasAttackable = GetOwner()->GetClass()->ImplementsInterface(UAttackableInterface::StaticClass());
	if(HasAttackable)
	{
		auto AttackableObject = Cast<IAttackableInterface>(GetOwner());

		// 마지막 콤보 이벤트 실행
		if(AttackableObject->IsFinalAttack())
		{
			bool HasHitEvent = HittedActor->GetClass()->ImplementsInterface(UHitEventInterface::StaticClass());
			if (HasHitEvent)
			{
				auto HitEventObject = Cast<IHitEventInterface>(HittedActor);
				HitEventObject->HitFinalAttack();
			}
		}
	}
}


// 공격 체크 함수
void UWeaponManagerComponent::AttackCheck()
{
	if (IsValid(ManagedWeapon))
	{
		EAttackType AttackType = ManagedWeapon->GetCurrentAttackType();

		// 공격 타입을 확인
		if (AttackType == EAttackType::Swing)
		{
			// 휘두르기 공격 체크 수행
			AttackCheckSwing();
		}
		
		if (AttackType == EAttackType::Smash)
		{
			// 내려찍기 공격 체크 수행
			AttackCheckSmash();
		}
	}
}

// 휘두르기 공격 체크
void UWeaponManagerComponent::AttackCheckSwing()
{
	// 현재 무기를 가지고 있는지 null 체크
	if (!IsValid(ManagedWeapon))
	{
		return;
	}

	// 소켓 정보를 가져올 메쉬 변수 선언
	auto WeaponMesh = ManagedWeapon->GetWeaponMesh();
	if (!IsValid(WeaponMesh))
	{
		return;
	}

	// 첫번째 수행인지 체크
	if (bFirstTake)
	{
		bFirstTake = false;

		// 현재 소켓의 위치를 이전 소켓 위치에 저장
		PrevStartLoc = WeaponMesh->GetSocketLocation("HitStart");
		PrevEndLoc = WeaponMesh->GetSocketLocation("HitEnd");
	}
	else
	{
		// 첫번째 이후 수행이라면
		// 현재 소켓 위치를 가져오기
		FVector CurrStartLoc = WeaponMesh->GetSocketLocation("HitStart");
		FVector CurrEndLoc = WeaponMesh->GetSocketLocation("HitEnd");

		// 이전 위치와 현재 위치의 소켓 정보로 사각형을 그리기 위해 계산
		FVector MidStartLoc = (CurrStartLoc + PrevStartLoc) * 0.5f;
		FVector MidEndLoc = (CurrEndLoc + PrevEndLoc) * 0.5f;
		float MidWidth = (CurrEndLoc - PrevEndLoc).Length() * 0.5f;

		// 박스를 그릴 방향
		FRotator BoxDirection = (CurrEndLoc - PrevEndLoc).Rotation();

		// 충돌 정보를 저장할 배열
		TArray<FHitResult> HitArray;

		// 중복 충돌 체크 방지
		HitIgnore.AddUnique(GetOwner());

		// BoxTraceMulti로 충돌 체크
		bool Hit = UKismetSystemLibrary::BoxTraceMulti(GetWorld(), MidStartLoc, MidEndLoc, FVector(MidWidth, 0.f, 0.f),
			BoxDirection, AttackChannel, false, HitIgnore, EDrawDebugTrace::None, HitArray, true
			, FLinearColor::Red, FLinearColor::Green, 0.5f
		);

		if (Hit)
		{
			for (const auto& HitResult : HitArray)
			{
				auto HittedActor = HitResult.GetActor();

				HitIgnore.AddUnique(HittedActor);

				// 타격감 함수 호출
				PlayAttackReaction(HittedActor);

				// 무기에서 피해량 정보 가져오기
				auto AttackDamage = ManagedWeapon->GetWeaponData()->AttackDamage;

				// 충돌한 물체에 피해를 주는 함수 실행
				UGameplayStatics::ApplyDamage(HittedActor, AttackDamage, HittedActor->GetInstigatorController(), GetOwner(), nullptr);

				// 피격 시 이펙트 생성
				FVector ParticleScale(0.75f, 0.75f, 0.75f);
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticle->Template,
					HitResult.ImpactPoint, FRotator::ZeroRotator, ParticleScale, true, EPSCPoolMethod::None, true);
			}
		}

		// 현재 소켓 위치를 이전 소켓으로 저장
		PrevStartLoc = CurrStartLoc;
		PrevEndLoc = CurrEndLoc;
	}
}


// 내려찍기 공격체크 함수
void UWeaponManagerComponent::AttackCheckSmash()
{
	if (!IsValid(ManagedWeapon))
	{
		return;
	}

	auto WeaponMesh = ManagedWeapon->GetWeaponMesh();
	if (!IsValid(WeaponMesh))
	{
		return;
	}

	// 소켓 2개의 중점을 충돌 구체의 원점으로 설정
	FVector StartLoc = WeaponMesh->GetSocketLocation("HitStart");
	FVector EndLoc = WeaponMesh->GetSocketLocation("HitEnd");
	FVector MidLoc = (StartLoc + EndLoc) * 0.5f;

	// 충돌 정보를 저장할 배열
	TArray<FHitResult> HitArray;

	// 중복 충돌 체크 방지
	HitIgnore.AddUnique(GetOwner());

	// SphereTraceMulti로 충돌 체크
	bool Hit = UKismetSystemLibrary::SphereTraceMulti(GetWorld(), MidLoc, MidLoc, 120.f,
		AttackChannel, false, HitIgnore, EDrawDebugTrace::None, HitArray, true
		, FLinearColor::Red, FLinearColor::Green, 0.5f
	);

	if (Hit)
	{
		for (const auto& HitResult : HitArray)
		{
			auto HittedActor = HitResult.GetActor();

			HitIgnore.AddUnique(HittedActor);
			
			// 타격감 함수 호출
			PlayAttackReaction(HittedActor);

			// 충돌한 물체에 피해를 주는 함수 실행
			auto AttackDamage = ManagedWeapon->GetWeaponData()->AttackDamage;
			UGameplayStatics::ApplyDamage(HittedActor, AttackDamage, HittedActor->GetInstigatorController(), GetOwner(), nullptr);

			// 피격 시 이펙트 생성
			FVector ParticleScale(0.75f, 0.75f, 0.75f);
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticle->Template,
				HittedActor->GetActorLocation(), FRotator::ZeroRotator, ParticleScale, true, EPSCPoolMethod::None, true);
		}
	}
}

// 공격 충돌 체크 종료 함수
void UWeaponManagerComponent::AttackCheckEnd()
{
	bFirstTake = true;
	HitIgnore.Empty();
}

// 무기 트레일 재생
void UWeaponManagerComponent::WeaponTrailBegin()
{
	if (IsValid(ManagedWeapon))
	{
		ManagedWeapon->WeaponTrailBegin();
	}
}

// 무기 트레일 중지
void UWeaponManagerComponent::WeaponTrailEnd()
{
	if (IsValid(ManagedWeapon))
	{
		ManagedWeapon->WeaponTrailEnd();
	}
}

// AI가 주우러 갈 무기를 탐색하는 함수
void UWeaponManagerComponent::FindWeapon(AWeapon*& TargetWeapon)
{
	if (!IsValid(TargetWeapon))
	{
		// 현재 타겟으로 삼은 무기가 없다면 탐색
		GetNearestWeapon(TargetWeapon, FindRadius);
	}
	else
	{
		// 탐색한 무기가 있다면 무기가 장착 가능한지 확인
		if (TargetWeapon->GetCanEquip())
		{
			// 탐색 성공
			return;
		}
		else
		{
			// 장착 불가능하다면 타겟 초기화
			TargetWeapon = nullptr;
		}
	}
}
