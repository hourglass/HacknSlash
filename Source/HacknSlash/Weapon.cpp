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

// 데이터 테이블에서 Id로 무기 정보를 가져오는 함수
void AWeapon::SetWeaponDataById(int Id)
{
	// 데이터 테이블을 가지고 있는 게임 인스턴스에 접근
	auto HnsGameInstance = Cast<UHnsGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));

	if (IsValid(HnsGameInstance))
	{
		// Id를 통해 무기 정보 가져오기
		WeaponData = HnsGameInstance->GetWeaponDataById(Id);
		if (WeaponData)
		{
			// 무기의 메쉬를 세팅
			Mesh->SetSkeletalMesh(WeaponData->WeaponMesh);

			// 무기의 최대 콤보 횟수를 세팅
			SetMaxComboBySection();
		}
	}
}

// 데이터 테이블에서 무기 이름으로 정보를 가져오는 함수
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

// 무기의 공격 타입을 설정하는 함수
void AWeapon::SetCurrentAttackType(EAttackType NewType)
{
	CurrentAttackType = NewType;
}

// 무기의 최대 콤보 횟수를 설정하는 함수
void AWeapon::SetMaxComboBySection()
{
	if (WeaponData->AttackMontage)
	{
		MaxCombo = WeaponData->AttackMontage->CompositeSections.Num();
	}
}

// 무기 장착 쿨타임 함수
void AWeapon::SetCanEquipByTimer()
{
	bCanEquip = false;

	FTimerHandle WaitHandle;
	float WaitTime = 0.25f;
	GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]() 
		{ 
			bCanEquip = true; 
		}
	), WaitTime, false);
}

// 트레일 재생 함수
void AWeapon::WeaponTrailBegin()
{
	WeaponTrail->BeginTrails(FName("TrailStart"), FName("TrailEnd"), ETrailWidthMode::ETrailWidthMode_FromCentre, 1.f);
}

// 트레일 종료 함수
void AWeapon::WeaponTrailEnd()
{
	WeaponTrail->EndTrails();
}

// 무기를 주웠을 때 발생하는 이벤트
void AWeapon::PickUpEvent()
{
	OnPickUp.Broadcast();
	OnPickUp.Clear();
}

// 멤버 변수 Get 함수
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