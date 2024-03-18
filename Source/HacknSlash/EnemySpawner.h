// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "HacknSlash.h"
#include "GameFramework/Actor.h"
#include "EnemySpawner.generated.h"

UCLASS()
class HACKNSLASH_API AEnemySpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEnemySpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SpawnEnemy();
	void SpawnEnemyByTimer();
	void SpawnWeapon();

	UBoxComponent* SpawnArea;
	FVector AreaSize;

private:
	TArray<AActor*> SpawnActors;

	FTimerHandle SpawnTimerHandle;
	int32 MaxSpawnCount;
	int32 SpawnCount;
};
