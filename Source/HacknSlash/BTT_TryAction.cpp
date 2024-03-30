// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_TryAction.h"

EBTNodeResult::Type UBTT_TryAction::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);
	
	// 동작간 랜덤한 텀을 주기 위해 수행하는 함수
	// 1/6 확률로 해당 동작을 수행할 지 결정
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
