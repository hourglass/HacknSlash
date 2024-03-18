// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"

AEnemyAIController::AEnemyAIController()
{
	Blackboard = CurrentBB;

	static ConstructorHelpers::FObjectFinder<UBlackboardData>
		BB_Data(TEXT("/Game/AI/BB_Enemy.BB_Enemy"));

	if (BB_Data.Succeeded())
	{
		EnemyBB = BB_Data.Object;
	}

	static ConstructorHelpers::FObjectFinder<UBehaviorTree>
		BT_Data(TEXT("/Game/AI/BT_Enemy.BT_Enemy"));

	if (BT_Data.Succeeded())
	{
		EnemyBT = BT_Data.Object;
	}
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	bool BB_Succeeded = UseBlackboard(EnemyBB, CurrentBB);
	if (!BB_Succeeded)
	{
		UE_LOG(LogTemp, Warning, TEXT("Set BB Error"));
	}

	bool BT_Succeeded = RunBehaviorTree(EnemyBT);
	if (!BT_Succeeded)
	{
		UE_LOG(LogTemp, Warning, TEXT("Run BT Error"));
	}
}

void AEnemyAIController::OnUnPossess()
{
	Super::OnUnPossess();

	Destroy();
}


// Set BlackBoard Key
void AEnemyAIController::SetState(uint8 State)
{
	CurrentBB->SetValueAsEnum(FName("EnemyState"), State);
}

void AEnemyAIController::SetTargetActor(APawn* TargetPawn)
{
	if (IsValid(TargetPawn))
	{
		CurrentBB->SetValueAsObject(FName("TargetActor"), TargetPawn);
	}
}

void AEnemyAIController::SetTargetWeapon(AActor* Weapon)
{
	if (IsValid(Weapon))
	{
		CurrentBB->SetValueAsObject(FName("TargetWeapon"), Weapon);
	}
}

void AEnemyAIController::SetCurrentWeapon(AActor* Weapon)
{
	if (IsValid(Weapon))
	{
		CurrentBB->SetValueAsObject(FName("CurrentWeapon"), Weapon);
	}
}

void AEnemyAIController::SetDestination(FVector Destination)
{
	CurrentBB->SetValueAsVector(FName("Destination"), Destination);
}


// Clear BlackBoard Key
void AEnemyAIController::ClearTargetActor()
{
	CurrentBB->ClearValue(FName("TargetActor"));
}

void AEnemyAIController::ClearTargetWeapon()
{
	CurrentBB->ClearValue(FName("TargetWeapon"));

}

void AEnemyAIController::ClearCurrentWeapon()
{
	CurrentBB->ClearValue(FName("CurrentWeapon"));
}

void AEnemyAIController::ClearDestination()
{
	CurrentBB->ClearValue(FName("Destination"));
}


// Get BlackBoard Data
AActor* AEnemyAIController::GetActorFromBB(FName KeyName)
{
	auto BBActor = Cast<AActor>(CurrentBB->GetValueAsObject(KeyName));
	return BBActor;
}



