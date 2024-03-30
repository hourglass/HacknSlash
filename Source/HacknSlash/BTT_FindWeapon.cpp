// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_FindWeapon.h"
#include "AIController.h"
#include "AIBehaviorInterface.h"

EBTNodeResult::Type UBTT_FindWeapon::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	auto MyPawn = OwnerComp.GetAIOwner()->GetPawn();

	// �ֿ췯 �� ���⸦ Ž���ϴ� �Լ�
	// ���� ���Ͱ� �������̽��� ������ �ִ��� Ȯ��
	bool HasAIBehavior = MyPawn->GetClass()->ImplementsInterface(UAIBehaviorInterface::StaticClass());
	if (HasAIBehavior)
	{
		// �������̽����� FindWeapon() �Լ� ����
		auto AIBehaviorObject = Cast<IAIBehaviorInterface>(MyPawn);
		AIBehaviorObject->FindWeapon();

		return  EBTNodeResult::Succeeded;
	}

	return  EBTNodeResult::Failed;
}