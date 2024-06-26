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

// ���� ���� �Լ�
void UWeaponManagerComponent::EquipWeapon(AWeapon*& Weapon)
{
	if (IsValid(Weapon))
	{
		return;
	}

	// �ֿ� �� �ִ� ���� ����� ���� Ž��
	GetNearestWeapon(Weapon, EquipRadius);

	// ���� Ž���� ����������
	if (IsValid(Weapon))	
	{
		// ������ ����� ����
		SetManagedWeapon(Weapon);

		// ���� ĳ������ �ڽ� ��ü�� ����
		if (IsValid(OwnCharacter))
		{
			// ���� �� ��ġ�� �ִ� ���Ͽ� ��ġ�ϵ��� ����
			Weapon->AttachToComponent(OwnCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, FName("Weapon"));
			Weapon->SetOwner(OwnCharacter);

			// �ٸ� ĳ���Ͱ� ���� ���⸦ ���� ���ϵ��� ����
			Weapon->SetCanEquip(false);

			// ���⸦ �ֿ��� �� �̺�Ʈ ����
			Weapon->PickUpEvent();
		}
	}
}

// ������ ���⸦ ����Ʈ���� �Լ�
void UWeaponManagerComponent::DropWeapon(AWeapon*& Weapon)
{ 
	if (IsValid(Weapon))
	{
		// Ʈ������ �ڽ� ���� ��ü
		Weapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform); 

		// ���� ���� ��Ÿ�� ����
		Weapon->SetCanEquipByTimer();

		// ���� Ʈ���� ����
		Weapon->WeaponTrailEnd();	

		// ����Ʈ�� ������ Ʈ������ ����
		FVector WeaponLoc = Weapon->GetActorLocation();
		Weapon->SetActorLocation(FVector(WeaponLoc.X, WeaponLoc.Y, 50.f));
		Weapon->SetActorRotation(FRotator::ZeroRotator);

		// ���� ���� ���� �ʱ�ȭ
		Weapon = nullptr;
		SetManagedWeapon(nullptr);
	}
}

// �ǰ� �� ���⸦ ����Ʈ���� �Լ�
void UWeaponManagerComponent::DropWeaponByHit(AWeapon*& Weapon)
{
	if (IsValid(Weapon))
	{
		// Ʈ������ �ڽ� ���� ��ü
		Weapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		// ���� ���� ��Ÿ�� ����
		Weapon->SetCanEquipByTimer();

		// ���� Ʈ���� ����
		Weapon->WeaponTrailEnd();	  

		// ����Ʈ�� ������ Ʈ������ ����
		FVector OwnerForward = GetOwner()->GetActorForwardVector();
		FVector WeaponLoc = Weapon->GetActorLocation() - (OwnerForward * 250.f);

		Weapon->SetActorLocation(FVector(WeaponLoc.X, WeaponLoc.Y, 50.f));
		Weapon->SetActorRotation(FRotator::ZeroRotator);

		// ���� ���� ���� �ʱ�ȭ
		Weapon = nullptr;
		SetManagedWeapon(nullptr);
	}
}


// ���� ����� ���⸦ Ž���ϴ� �Լ�
void UWeaponManagerComponent::GetNearestWeapon(AWeapon*& Weapon, float SensingRadius)
{
	// �� ������Ʈ�� ���� ĳ������ �����̼� ��������
	FVector CharterLoc = GetOwner()->GetActorLocation();

	// ���� Ž�� �浹 üũ ä�� ����
	ETraceTypeQuery WeaponSensingChannel =
		UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel3);

	// �浹�� ������ ���� ����
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Init(GetOwner(), 1);

	// �浹 ���� �� ������ ������ �迭
	TArray<FHitResult> HitArray;

	// SphereTraceMulti �浹 üũ
	bool Hit = UKismetSystemLibrary::SphereTraceMulti(GetWorld(), CharterLoc, CharterLoc, SensingRadius,
		WeaponSensingChannel, false, IgnoreActors, EDrawDebugTrace::None, HitArray, true);

	// �浹 ���� ��
	if (Hit)
	{
		// Ž���� ���⸦ ������ ����
		AWeapon* NearestWeapon = nullptr;

		// ������ �� �ּҰ� ����
		float MinDistance = SensingRadius;

		// �迭�� ��ȸ�ϸ� ���� ����� ���⸦ Ž��
		for (const auto& HitResult : HitArray)
		{
			auto DetectedWeapon = Cast<AWeapon>(HitResult.GetActor());
			if (IsValid(DetectedWeapon))
			{
				// ���Ⱑ ���� �Ұ����� ���¶�� ���� ���� Ž��
				if (!DetectedWeapon->GetCanEquip())
				{
					continue;
				}

				// ����� ĳ������ �Ÿ� ����� �ּҰ� ����
				float Distance = DetectedWeapon->GetDistanceTo(GetOwner());
				if (Distance <= MinDistance)
				{
					NearestWeapon = DetectedWeapon;
					MinDistance = Distance;
				}
			}
		}

		// ���� ����� ���⸦ ������ ����� ����
		Weapon = NearestWeapon;
	}
	else
	{
		return;
	}
}


// ���� �� Ÿ�ݰ��� ���� �Լ� ����
void UWeaponManagerComponent::PlayAttackReaction(AActor* HittedActor)
{
	// Play Attack Reation
	PlayCameraShake();
	PlayAttackStiffen();
	LaunchTarget(HittedActor);
	FinalAttackEvent(HittedActor);
}

// ī�޶� ����ũ ���� �Լ�
void UWeaponManagerComponent::PlayCameraShake()
{
	// ���⿡ ����� ī�޶� ����ũ ���� ���� ��������
	HitShake = ManagedWeapon->GetWeaponData()->HitShakeWeak;
	if (IsValid(HitShake))
	{
		// �������� ������ ī�޶� ����ũ ����
		UGameplayStatics::PlayWorldCameraShake(GetWorld(), HitShake, GetOwner()->GetActorLocation(), 0, 1000, 1.f, false);
	}
}

// ���� ���� �� �ִϸ��̼��� ���������� ������ ����� �Լ�
void UWeaponManagerComponent::PlayAttackStiffen()
{
	if (IsValid(OwnCharacter) && IsValid(ManagedWeapon))
	{
		// ĳ������ �ִ� �ν��Ͻ��� ������ ��Ÿ�� �ӵ��� ����
		// ���� ������ �ִ� �������� ��ġ�� ����
		OwnCharacter->GetMesh()->GetAnimInstance()->Montage_SetPlayRate(
			ManagedWeapon->GetWeaponData()->AttackMontage,
			ManagedWeapon->GetWeaponData()->AttackStiffness
		);

		// ���� ������ �ִ� ���� �ӵ� ���� �ð��� ��������
		auto AttackRecovery = ManagedWeapon->GetWeaponData()->AttackRecovery;

		// Ÿ�̸Ӹ� ���� ���� �ð��Ŀ� ���� �ӵ��� ����
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

// ���ݿ� ���� Ÿ���� �ڷ� �˹��Ű�� �Լ�
void UWeaponManagerComponent::LaunchTarget(AActor* HittedActor)
{
	if (IsValid(ManagedWeapon))
	{
		auto Target = Cast<ACharacter>(HittedActor);
		if (Target)
		{
			// ���� ����� ���ϴ� ���� ���ϱ�
			auto OwnerActor = GetOwner();
			auto TargetVector = Target->GetActorLocation() - OwnerActor->GetActorLocation();

			// ������ �������� ������ ����� ĳ������ �ڿ� �ִ��� Ȯ��
			auto TargetDir = FVector::DotProduct(TargetVector, OwnerActor->GetActorForwardVector());

			// cos���� 0���� ũ�� ����, ������ �Ĺ�
			float Direction;
			(TargetDir >= 0) ? Direction = 1.f : Direction = -1.f;

			// ĳ���� �� ���� ���� �ڷ� �з������� ����
			auto FowardVector = OwnerActor->GetActorForwardVector();

			// ���⿡�� �˹� ��ų ��ġ�� ��������
			float LaunchForce = ManagedWeapon->GetWeaponData()->LaunchForce * Direction;

			// �˹� ����
			Target->LaunchCharacter(FowardVector * LaunchForce, false, false);
		}
	}
}

// ������ �޺� �ÿ� �߻��� �̺�Ʈ
void UWeaponManagerComponent::FinalAttackEvent(AActor* HittedActor)
{ 
	// ���� ������ ��ü�� ����ϴ� �������̽� ��������
	bool HasAttackable = GetOwner()->GetClass()->ImplementsInterface(UAttackableInterface::StaticClass());
	if(HasAttackable)
	{
		auto AttackableObject = Cast<IAttackableInterface>(GetOwner());

		// ������ �޺� �̺�Ʈ ����
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


// ���� üũ �Լ�
void UWeaponManagerComponent::AttackCheck()
{
	if (IsValid(ManagedWeapon))
	{
		EAttackType AttackType = ManagedWeapon->GetCurrentAttackType();

		// ���� Ÿ���� Ȯ��
		if (AttackType == EAttackType::Swing)
		{
			// �ֵθ��� ���� üũ ����
			AttackCheckSwing();
		}
		
		if (AttackType == EAttackType::Smash)
		{
			// ������� ���� üũ ����
			AttackCheckSmash();
		}
	}
}

// �ֵθ��� ���� üũ
void UWeaponManagerComponent::AttackCheckSwing()
{
	// ���� ���⸦ ������ �ִ��� null üũ
	if (!IsValid(ManagedWeapon))
	{
		return;
	}

	// ���� ������ ������ �޽� ���� ����
	auto WeaponMesh = ManagedWeapon->GetWeaponMesh();
	if (!IsValid(WeaponMesh))
	{
		return;
	}

	// ù��° �������� üũ
	if (bFirstTake)
	{
		bFirstTake = false;

		// ���� ������ ��ġ�� ���� ���� ��ġ�� ����
		PrevStartLoc = WeaponMesh->GetSocketLocation("HitStart");
		PrevEndLoc = WeaponMesh->GetSocketLocation("HitEnd");
	}
	else
	{
		// ù��° ���� �����̶��
		// ���� ���� ��ġ�� ��������
		FVector CurrStartLoc = WeaponMesh->GetSocketLocation("HitStart");
		FVector CurrEndLoc = WeaponMesh->GetSocketLocation("HitEnd");

		// ���� ��ġ�� ���� ��ġ�� ���� ������ �簢���� �׸��� ���� ���
		FVector MidStartLoc = (CurrStartLoc + PrevStartLoc) * 0.5f;
		FVector MidEndLoc = (CurrEndLoc + PrevEndLoc) * 0.5f;
		float MidWidth = (CurrEndLoc - PrevEndLoc).Length() * 0.5f;

		// �ڽ��� �׸� ����
		FRotator BoxDirection = (CurrEndLoc - PrevEndLoc).Rotation();

		// �浹 ������ ������ �迭
		TArray<FHitResult> HitArray;

		// �ߺ� �浹 üũ ����
		HitIgnore.AddUnique(GetOwner());

		// BoxTraceMulti�� �浹 üũ
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

				// Ÿ�ݰ� �Լ� ȣ��
				PlayAttackReaction(HittedActor);

				// ���⿡�� ���ط� ���� ��������
				auto AttackDamage = ManagedWeapon->GetWeaponData()->AttackDamage;

				// �浹�� ��ü�� ���ظ� �ִ� �Լ� ����
				UGameplayStatics::ApplyDamage(HittedActor, AttackDamage, HittedActor->GetInstigatorController(), GetOwner(), nullptr);

				// �ǰ� �� ����Ʈ ����
				FVector ParticleScale(0.75f, 0.75f, 0.75f);
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticle->Template,
					HitResult.ImpactPoint, FRotator::ZeroRotator, ParticleScale, true, EPSCPoolMethod::None, true);
			}
		}

		// ���� ���� ��ġ�� ���� �������� ����
		PrevStartLoc = CurrStartLoc;
		PrevEndLoc = CurrEndLoc;
	}
}


// ������� ����üũ �Լ�
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

	// ���� 2���� ������ �浹 ��ü�� �������� ����
	FVector StartLoc = WeaponMesh->GetSocketLocation("HitStart");
	FVector EndLoc = WeaponMesh->GetSocketLocation("HitEnd");
	FVector MidLoc = (StartLoc + EndLoc) * 0.5f;

	// �浹 ������ ������ �迭
	TArray<FHitResult> HitArray;

	// �ߺ� �浹 üũ ����
	HitIgnore.AddUnique(GetOwner());

	// SphereTraceMulti�� �浹 üũ
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
			
			// Ÿ�ݰ� �Լ� ȣ��
			PlayAttackReaction(HittedActor);

			// �浹�� ��ü�� ���ظ� �ִ� �Լ� ����
			auto AttackDamage = ManagedWeapon->GetWeaponData()->AttackDamage;
			UGameplayStatics::ApplyDamage(HittedActor, AttackDamage, HittedActor->GetInstigatorController(), GetOwner(), nullptr);

			// �ǰ� �� ����Ʈ ����
			FVector ParticleScale(0.75f, 0.75f, 0.75f);
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticle->Template,
				HittedActor->GetActorLocation(), FRotator::ZeroRotator, ParticleScale, true, EPSCPoolMethod::None, true);
		}
	}
}

// ���� �浹 üũ ���� �Լ�
void UWeaponManagerComponent::AttackCheckEnd()
{
	bFirstTake = true;
	HitIgnore.Empty();
}

// ���� Ʈ���� ���
void UWeaponManagerComponent::WeaponTrailBegin()
{
	if (IsValid(ManagedWeapon))
	{
		ManagedWeapon->WeaponTrailBegin();
	}
}

// ���� Ʈ���� ����
void UWeaponManagerComponent::WeaponTrailEnd()
{
	if (IsValid(ManagedWeapon))
	{
		ManagedWeapon->WeaponTrailEnd();
	}
}

// AI�� �ֿ췯 �� ���⸦ Ž���ϴ� �Լ�
void UWeaponManagerComponent::FindWeapon(AWeapon*& TargetWeapon)
{
	if (!IsValid(TargetWeapon))
	{
		// ���� Ÿ������ ���� ���Ⱑ ���ٸ� Ž��
		GetNearestWeapon(TargetWeapon, FindRadius);
	}
	else
	{
		// Ž���� ���Ⱑ �ִٸ� ���Ⱑ ���� �������� Ȯ��
		if (TargetWeapon->GetCanEquip())
		{
			// Ž�� ����
			return;
		}
		else
		{
			// ���� �Ұ����ϴٸ� Ÿ�� �ʱ�ȭ
			TargetWeapon = nullptr;
		}
	}
}
