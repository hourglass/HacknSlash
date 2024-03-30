// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_EquipWeapon.h"
#include "AIController.h"
#include "AIBehaviorInterface.h"

EBTNodeResult::Type UBTT_EquipWeapon::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	auto MyPawn = OwnerComp.GetAIOwner()->GetPawn();

	// 근처에 떨어진 무기를 장착하는 함수
	// 현재 액터가 인터페이스를 가지고 있는지 확인
	bool HasAIBehavior = MyPawn->GetClass()->ImplementsInterface(UAIBehaviorInterface::StaticClass());
	if (HasAIBehavior)
	{
		// 인터페이스에서 FindWeapon() 함수 수행
		auto AIBehaviorObject = Cast<IAIBehaviorInterface>(MyPawn);
		AIBehaviorObject->EquipWeapon();

		return  EBTNodeResult::Succeeded;
	}

	return  EBTNodeResult::Failed;
}