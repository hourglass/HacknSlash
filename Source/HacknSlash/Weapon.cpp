//Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "HnsGameInstance.h"

// Sets default values
AWeapon::AWeapon()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Variable Setting //
	bCanEquip = true;

	// Component Setting //
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SK_Mesh"));
	Mesh->SetupAttachment(RootComponent);

	WeaponCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("WeaponCollision"));
	WeaponCollision->SetRelativeLocation(FVector(0.f, 0.f, 50.f));
	WeaponCollision->SetBoxExtent(FVector(10.f, 10.f, 50.f));
	WeaponCollision->SetCollisionProfileName(TEXT("Weapon"));
	WeaponCollision->SetupAttachment(Mesh);

	WeaponTrail = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("WeaponTrail"));
	WeaponTrail->SetupAttachment(Mesh);

	static ConstructorHelpers::FObjectFinder<UParticleSystem> 
		TrailEffect(TEXT("/Game/InfinityBladeMagicFX/particles/P_ky_trail_fire.P_ky_trail_fire"));

	auto result = TrailEffect.Succeeded();
	if (TrailEffect.Succeeded())
	{
		WeaponTrail->SetTemplate(TrailEffect.Object);
	}
}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::SetWeaponDataById(int Id)
{
	auto HnsGameInstance = Cast<UHnsGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));

	if (IsValid(HnsGameInstance))
	{
		WeaponData = HnsGameInstance->GetWeaponDataById(Id);
		if (WeaponData)
		{
			Mesh->SetSkeletalMesh(WeaponData->WeaponMesh);
			SetMaxComboBySection();
		}
	}
}

void AWeapon::SetWeaponDataByName(FName WeaponName)
{
	auto HnsGameInstance = Cast<UHnsGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));

	if (IsValid(HnsGameInstance))
	{
		WeaponData = HnsGameInstance->GetWeaponDataByName(WeaponName);
		if (WeaponData)
		{
			Mesh->SetSkeletalMesh(WeaponData->WeaponMesh);
			SetMaxComboBySection();
		}
	}
}

void AWeapon::SetCurrentAttackType(EAttackType NewType)
{
	CurrentAttackType = NewType;
}

void AWeapon::SetMaxComboBySection()
{
	if (WeaponData->AttackMontage)
	{
		MaxCombo = WeaponData->AttackMontage->CompositeSections.Num();
	}
}

void AWeapon::SetCanEquipByTimer()
{
	bCanEquip = false;

	FTimerHandle WaitHandle;
	float WaitTime = 0.25f;
	GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]() {

		bCanEquip = true;
		}), WaitTime, false);
}

void AWeapon::SetCanEquip(bool State)
{
	bCanEquip = State;
}

FWeaponData* AWeapon::GetWeaponData()
{
	return WeaponData;
}

USkeletalMeshComponent* AWeapon::GetWeaponMesh()
{
	return Mesh;
}

EAttackType AWeapon::GetCurrentAttackType()
{
	return CurrentAttackType;
}

int32 AWeapon::GetMaxCombo()
{
	return MaxCombo;
}

bool AWeapon::GetCanEquip()
{
	return bCanEquip;
}

void AWeapon::WeaponTrailBegin()
{
	WeaponTrail->BeginTrails(FName("TrailStart"), FName("TrailEnd"), ETrailWidthMode::ETrailWidthMode_FromCentre, 1.f);
}

void AWeapon::WeaponTrailEnd()
{
	WeaponTrail->EndTrails();
}

void AWeapon::PickUpEvent()
{
	OnPickUp.Broadcast();
	OnPickUp.Clear();
}