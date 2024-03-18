// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_TryAction.h"

EBTNodeResult::Type UBTT_TryAction::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	int32 RandomNumber = FMath::RandRange(1, 6);
	if (RandomNumber % 6 == 0)
	{
		return  EBTNodeResult::Succeeded;
	}
	else
	{
		return  EBTNodeResult::Failed;
	}
}
