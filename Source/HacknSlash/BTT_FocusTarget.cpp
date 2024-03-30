// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_FocusTarget.h"
#include "AIController.h"
#include "AIBehaviorInterface.h"

EBTNodeResult::Type UBTT_FocusTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	auto MyPawn = OwnerComp.GetAIOwner()->GetPawn();

	// �ٶ� ��ǥ�� �����ϴ� �Լ�
	// ���� ���Ͱ� �������̽��� ������ �ִ��� Ȯ��
	bool HasAIBehavior = MyPawn->GetClass()->ImplementsInterface(UAIBehaviorInterface::StaticClass());
	if (HasAIBehavior)
	{
		// IAIBehavior �������̽��� ĳ����
		auto AIBehaviorObject = Cast<IAIBehaviorInterface>(MyPawn);

		// Behavior Tree�� �����带 �������� �Լ�
		// �����忡 ��ϵ� Ÿ���� ��������
		auto TargetActor = AIBehaviorObject->GetActorFromBB(Target.SelectedKeyName);
		if (TargetActor)
		{
			// AAIController ĳ����
			auto AIController = Cast<AAIController>(OwnerComp.GetOwner());
			if (AIController)
			{
				// ��Ʈ�ѷ��� ���� Ÿ���� �ٶ󺸵��� ����
				AIController->SetFocus(TargetActor);

				return EBTNodeResult::Succeeded;
			}
		}
	}

	return EBTNodeResult::Failed;
}
