// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemySpawner.h"
#include "Enemy.h"
#include "Weapon.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AEnemySpawner::AEnemySpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Variable Setting //
	MaxSpawnCount = 5;
	SpawnCount = 0;

	// Component Setting //
	SpawnArea = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnArea"));
	AreaSize = FVector(500.f, 500.f, 50.f);
	SpawnArea->SetBoxExtent(AreaSize);
}

// Called when the game starts or when spawned
void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();
	
	SpawnWeapon();
	//SpawnEnemy();
}

// Called every frame
void AEnemySpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEnemySpawner::SpawnEnemy()
{
	if (SpawnCount >= MaxSpawnCount)
	{
		GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
		return;
	}

	auto Loc = UKismetMathLibrary::RandomPointInBoundingBox(GetActorLocation(), AreaSize);
	const FRotator rot = FRotator(0.f, 180.f, 0.f);

	auto NewEnemy = GetWorld()->SpawnActor<AEnemy>(AEnemy::StaticClass(), Loc, rot);
	if (IsValid(NewEnemy))
	{
		// SpawnActor로 Character/Pawn 생성 시 CharacterMovement가 작동하지 않음.
		// 아래 함수를 호출해줘야 컨트롤러 설정이 제대로 되면서 동작함.
		NewEnemy->SpawnDefaultController();

		auto NewWeapon = GetWorld()->SpawnActor<AWeapon>(AWeapon::StaticClass());
		if (NewWeapon)
		{
			int index = FMath::RandRange(0, 2);
			NewWeapon->SetWeaponDataById(index);
			if (NewWeapon->GetWeaponData() != nullptr)
			{
				NewWeapon->AttachToComponent(NewEnemy->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, FName("Weapon"));
				NewWeapon->SetOwner(NewEnemy);
				NewEnemy->SetCurrentWeapon(NewWeapon);
			}
		}

		SpawnActors.Add(NewEnemy);
	}
}

void AEnemySpawner::SpawnEnemyByTimer()
{
	float WaitTime = 0.2f;
	GetWorld()->GetTimerManager().SetTimer(SpawnTimerHandle, FTimerDelegate::CreateLambda([&]() {

		SpawnEnemy();
		SpawnCount++;
		}), WaitTime, true, WaitTime);
}

void AEnemySpawner::SpawnWeapon()
{
	const FVector Loc(GetActorLocation().X, GetActorLocation().Y, 50.f);

	auto NewWeapon = GetWorld()->SpawnActor<AWeapon>(AWeapon::StaticClass(), Loc, FRotator::ZeroRotator);
	if (IsValid(NewWeapon))
	{
		int index = FMath::RandRange(0,2);
		NewWeapon->SetWeaponDataById(index);
		NewWeapon->OnPickUp.AddLambda([this]()->void {

			SpawnEnemyByTimer();
			});
	}
}
