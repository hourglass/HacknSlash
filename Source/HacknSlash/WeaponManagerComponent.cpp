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


void UWeaponManagerComponent::EquipWeapon(AWeapon*& Weapon)
{
	if (IsValid(Weapon))
	{
		return;
	}

	GetNearestWeapon(Weapon, EquipRadius);
	if (IsValid(Weapon))	
	{
		SetManagedWeapon(Weapon);

		if (IsValid(OwnCharacter))
		{
			Weapon->AttachToComponent(OwnCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, FName("Weapon"));
			Weapon->SetOwner(OwnCharacter);
			Weapon->SetCanEquip(false);
			Weapon->PickUpEvent();
		}
	}
}

void UWeaponManagerComponent::FindWeapon(AWeapon*& Weapon)
{
	if (IsValid(Weapon))
	{
		if (Weapon->GetCanEquip())
		{
			return;
		}
		else
		{
			Weapon = nullptr;
		}
	}
	else
	{
		GetNearestWeapon(Weapon, FindRadius);
	}
}

void UWeaponManagerComponent::DropWeapon(AWeapon*& Weapon)
{ 
	if (IsValid(Weapon))
	{
		Weapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		Weapon->SetCanEquipByTimer();
		Weapon->WeaponTrailEnd();

		FVector WeaponLoc = Weapon->GetActorLocation();
		Weapon->SetActorLocation(FVector(WeaponLoc.X, WeaponLoc.Y, 50.f));
		Weapon->SetActorRotation(FRotator::ZeroRotator);

		Weapon = nullptr;
		SetManagedWeapon(nullptr);
	}
}

void UWeaponManagerComponent::HittedDropWeapon(AWeapon*& Weapon)
{
	if (IsValid(Weapon))
	{
		Weapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		Weapon->SetCanEquipByTimer();
		Weapon->WeaponTrailEnd();

		FVector OwnerForward = GetOwner()->GetActorForwardVector();
		FVector WeaponLoc = Weapon->GetActorLocation() - (OwnerForward * 250.f);

		Weapon->SetActorLocation(FVector(WeaponLoc.X, WeaponLoc.Y, 50.f));
		Weapon->SetActorRotation(FRotator::ZeroRotator);
		Weapon = nullptr;
		SetManagedWeapon(nullptr);
	}
}

void UWeaponManagerComponent::GetNearestWeapon(AWeapon*& Weapon, float SensingRadius)
{
	FVector CharterLoc = GetOwner()->GetActorLocation();

	ETraceTypeQuery WeaponSensingChannel =
		UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel3);

	TArray<AActor*> IgnoreActors;
	IgnoreActors.Init(GetOwner(), 1);

	TArray<FHitResult> HitArray;

	bool Hit = UKismetSystemLibrary::SphereTraceMulti(GetWorld(), CharterLoc, CharterLoc, SensingRadius,
		WeaponSensingChannel, false, IgnoreActors, EDrawDebugTrace::None, HitArray, true);

	if (Hit)
	{
		AWeapon* NearestWeapon = nullptr;
		float MinDistance = SensingRadius;

		for (const auto& HitResult : HitArray)
		{
			auto DetectedWeapon = Cast<AWeapon>(HitResult.GetActor());
			if (IsValid(DetectedWeapon))
			{
				if (!DetectedWeapon->GetCanEquip())
				{
					continue;
				}

				float Distance = DetectedWeapon->GetDistanceTo(GetOwner());
				if (Distance <= MinDistance)
				{
					NearestWeapon = DetectedWeapon;
					MinDistance = Distance;
				}
			}
		}

		Weapon = NearestWeapon;
	}
	else
	{
		return;
	}
}


void UWeaponManagerComponent::PlayCameraShake()
{
	HitShake = ManagedWeapon->GetWeaponData()->HitShakeWeak;
	if (IsValid(HitShake))
	{
		UGameplayStatics::PlayWorldCameraShake(GetWorld(), HitShake, GetOwner()->GetActorLocation(), 0, 1000, 1.f, false);
	}
}

void UWeaponManagerComponent::PlayAttackStiffen()
{
	if (IsValid(OwnCharacter) && IsValid(ManagedWeapon))
	{
		OwnCharacter->GetMesh()->GetAnimInstance()->Montage_SetPlayRate(
			ManagedWeapon->GetWeaponData()->AttackMontage,
			ManagedWeapon->GetWeaponData()->AttackStiffness
		);

		auto AttackRecovery = ManagedWeapon->GetWeaponData()->AttackRecovery;
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

void UWeaponManagerComponent::LaunchTarget(AActor* HittedActor)
{
	if (IsValid(ManagedWeapon))
	{
		auto Target = Cast<ACharacter>(HittedActor);
		if (Target)
		{
			auto OwnerActor = GetOwner();
			auto TargetVector = Target->GetActorLocation() - OwnerActor->GetActorLocation();
			auto TargetDir = FVector::DotProduct(TargetVector, OwnerActor->GetActorForwardVector());

			float Direction;
			(TargetDir >= 0) ? Direction = 1.f : Direction = -1.f;

			float LaunchForce = ManagedWeapon->GetWeaponData()->LaunchForce * Direction;
			auto FowardVector = OwnerActor->GetActorForwardVector();

			Target->LaunchCharacter(FowardVector * LaunchForce, false, false);
		}
	}
}

void UWeaponManagerComponent::FinalAttackEvent(AActor* HittedActor)
{ 
	bool HasAttackable = GetOwner()->GetClass()->ImplementsInterface(UAttackableInterface::StaticClass());
	if(HasAttackable)
	{
		auto AttackableObject = Cast<IAttackableInterface>(GetOwner());
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

void UWeaponManagerComponent::PlayAttackReaction(AActor* HittedActor)
{
	// Play Attack Reation
	PlayAttackStiffen();
	PlayCameraShake();
	LaunchTarget(HittedActor);
	FinalAttackEvent(HittedActor);
}


void UWeaponManagerComponent::AttackCheck()
{
	if (IsValid(ManagedWeapon))
	{
		EAttackType AttackType = ManagedWeapon->GetCurrentAttackType();

		if (AttackType == EAttackType::Swing)
		{
			AttackCheckSwing();
		}
		
		if (AttackType == EAttackType::Smash)
		{
			AttackCheckSmash();
		}
	}
}

void UWeaponManagerComponent::AttackCheckSwing()
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

	if (bFirstTake)
	{
		bFirstTake = false;
		PrevStartLoc = WeaponMesh->GetSocketLocation("HitStart");
		PrevEndLoc = WeaponMesh->GetSocketLocation("HitEnd");
	}
	else
	{
		FVector CurrStartLoc = WeaponMesh->GetSocketLocation("HitStart");
		FVector CurrEndLoc = WeaponMesh->GetSocketLocation("HitEnd");

		FVector MidStartLoc = (CurrStartLoc + PrevStartLoc) * 0.5f;
		FVector MidEndLoc = (CurrEndLoc + PrevEndLoc) * 0.5f;
		float MidWidth = (CurrEndLoc - PrevEndLoc).Length() * 0.5f;

		FRotator BoxDirection = (CurrEndLoc - PrevEndLoc).Rotation();

		TArray<FHitResult> HitArray;

		HitIgnore.AddUnique(GetOwner());

		bool Hit = UKismetSystemLibrary::BoxTraceMulti(GetWorld(), MidStartLoc, MidEndLoc, FVector(MidWidth, 0.f, 0.f),
			BoxDirection, AttackChannel, false, HitIgnore, EDrawDebugTrace::ForDuration, HitArray, true
			, FLinearColor::Red, FLinearColor::Green, 0.5f
		);

		if (Hit)
		{
			for (const auto& HitResult : HitArray)
			{
				auto HittedActor = HitResult.GetActor();

				HitIgnore.AddUnique(HittedActor);
				PlayAttackReaction(HittedActor);

				auto AttackDamage = ManagedWeapon->GetWeaponData()->AttackDamage;
				UGameplayStatics::ApplyDamage(HittedActor, AttackDamage, HittedActor->GetInstigatorController(), GetOwner(), nullptr);

				FVector ParticleScale(0.75f, 0.75f, 0.75f);
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticle->Template,
					HitResult.ImpactPoint, FRotator::ZeroRotator, ParticleScale, true, EPSCPoolMethod::None, true);
			}
		}

		PrevStartLoc = CurrStartLoc;
		PrevEndLoc = CurrEndLoc;
	}
}

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

	FVector StartLoc = WeaponMesh->GetSocketLocation("HitStart");
	FVector EndLoc = WeaponMesh->GetSocketLocation("HitEnd");
	FVector MidLoc = (StartLoc + EndLoc) * 0.5f;

	TArray<FHitResult> HitArray;

	HitIgnore.AddUnique(GetOwner());

	bool Hit = UKismetSystemLibrary::SphereTraceMulti(GetWorld(), MidLoc, MidLoc, 120.f,
		AttackChannel, false, HitIgnore, EDrawDebugTrace::ForDuration, HitArray, true
		, FLinearColor::Red, FLinearColor::Green, 0.5f
	);

	if (Hit)
	{
		for (const auto& HitResult : HitArray)
		{
			auto HittedActor = HitResult.GetActor();

			HitIgnore.AddUnique(HittedActor);
			PlayAttackReaction(HittedActor);

			auto AttackDamage = ManagedWeapon->GetWeaponData()->AttackDamage;
			UGameplayStatics::ApplyDamage(HittedActor, AttackDamage, HittedActor->GetInstigatorController(), GetOwner(), nullptr);

			FVector ParticleScale(0.75f, 0.75f, 0.75f);
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticle->Template,
				HittedActor->GetActorLocation(), FRotator::ZeroRotator, ParticleScale, true, EPSCPoolMethod::None, true);
		}
	}
}

void UWeaponManagerComponent::AttackCheckEnd()
{
	bFirstTake = true;
	HitIgnore.Empty();
}


void UWeaponManagerComponent::WeaponTrailBegin()
{
	if (IsValid(ManagedWeapon))
	{
		ManagedWeapon->WeaponTrailBegin();
	}
}

void UWeaponManagerComponent::WeaponTrailEnd()
{
	if (IsValid(ManagedWeapon))
	{
		ManagedWeapon->WeaponTrailEnd();
	}
}
