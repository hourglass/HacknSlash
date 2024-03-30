// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_FindWeapon.h"
#include "AIController.h"
#include "AIBehaviorInterface.h"

EBTNodeResult::Type UBTT_FindWeapon::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	auto MyPawn = OwnerComp.GetAIOwner()->GetPawn();

	// 주우러 갈 무기를 탐색하는 함수
	// 현재 액터가 인터페이스를 가지고 있는지 확인
	bool HasAIBehavior = MyPawn->GetClass()->ImplementsInterface(UAIBehaviorInterface::StaticClass());
	if (HasAIBehavior)
	{
		// 인터페이스에서 FindWeapon() 함수 수행
		auto AIBehaviorObject = Cast<IAIBehaviorInterface>(MyPawn);
		AIBehaviorObject->FindWeapon();

		return  EBTNodeResult::Succeeded;
	}

	return  EBTNodeResult::Failed;
}