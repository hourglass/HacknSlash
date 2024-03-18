// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

/**
 * 
 */
UCLASS()
class HACKNSLASH_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	AEnemyAIController();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	void SetState(uint8 State);
	void SetTargetActor(APawn* TargetPawn);
	void SetTargetWeapon(AActor* Weapon);
	void SetCurrentWeapon(AActor* Weapon);
	void SetDestination(FVector Destination);

	void ClearTargetActor();
	void ClearTargetWeapon();
	void ClearCurrentWeapon();
	void ClearDestination();

	AActor* GetActorFromBB(FName KeyName);

private:
	UPROPERTY()
	class UBehaviorTree* EnemyBT;

	UPROPERTY()
	class UBlackboardData* EnemyBB;

	UPROPERTY()
	class UBlackboardComponent* CurrentBB;
};
