// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_EquipWeapon.h"
#include "AIController.h"
#include "AIBehaviorInterface.h"

EBTNodeResult::Type UBTT_EquipWeapon::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	auto MyPawn = OwnerComp.GetAIOwner()->GetPawn();

	bool HasAIBehavior = MyPawn->GetClass()->ImplementsInterface(UAIBehaviorInterface::StaticClass());
	if (HasAIBehavior)
	{
		auto AIBehaviorObject = Cast<IAIBehaviorInterface>(MyPawn);
		AIBehaviorObject->EquipWeapon();

		return  EBTNodeResult::Succeeded;
	}

	return  EBTNodeResult::Failed;
}